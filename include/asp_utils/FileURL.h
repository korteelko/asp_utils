/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * FileURL *
 *   В файле описан функционал оборачивающий адрессацию данных
 * из внешних источников
 * ===================================================================
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__FILEURL_H
#define UTILS__FILEURL_H

#include "asp_utils/Base.h"
#include "asp_utils/Common.h"
#include "asp_utils/ErrorWrap.h"

#include <cassert>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

namespace asp_utils {
namespace file_utils {
/* todo: create class file_utils exception */

/**
 * \brief тип задания адреса файла, буффера памяти и т.п.
 * */
enum class url_t {
  empty = 0,
  /**
   * \brief путь в файловой системе
   * */
  fs_path = 1
};

/**
 * \brief Концепт проверки типа на соответствие допустимости
 *   использования типа для задания пути
 * \todo add wstring support
 * */
template <typename T>
concept PathType =
    std::is_same<T, fs::path>::value || std::is_same<T, std::string>::value;

/**
 * \brief Сетап URL - хост, юзер и т.п., все входные
 *   данные для фабрики URL объектов короч
 * \tparam PathT Тип пути, может быть fs::path, std::string, std::wstring
 * \note Сейчас то это просто для красоты
 * \todo replace 'root' type with struct wth overloaded operator[]
 *   for c++17 comp
 * */
template <PathType PathT>
struct SetupURLSample {
 public:
  SetupURLSample(url_t url_type, const PathT& root)
      : url_type(url_type), root(root) {}
  /**
   * \brief Полный префикс для пути(от начала до рута)
   * */
  inline PathT GetFullPrefix() const { return root; }
  /**
   * \brief Тип ссылки
   * */
  inline url_t GetURLType() const { return url_type; }

 public:
  /**
   * \brief Тип адресов для создания полных путей
   * */
  url_t url_type;
  /**
   * \brief Путь к корню системы(для файловой системы)
   * */
  PathT root;
};
using SetupURL = SetupURLSample<std::string>;

template <PathType PathT>
class FileURLRootSample;
/**
 * \brief Класс пути(м.б. обычный пути в файловой системе, урл,
 *   запрос к дб и т.п.)
 * \tparam PathT Тип пути, может быть fs::path, std::string, std::wstring
 * \note Немножко ликбеза:
 *   вид File(URI):
 *     "file://<host>/<path>"
 *   А вот обощённый URL несёт много больше информации:
 *     "<схема>:[//[<логин>[:<пароль>]@]<хост>[:<порт>]][/<URL‐путь>][?<параметры>][#<якорь>]"
 *   Естественно, полный url нам пока не нужен, но, лучше ввести лишнюю
 *     структуру хранящую все параметры подключения(хост, юзер с поролем),
 *     которую будем хранить в фабрике
 * */
template <PathType PathT>
class FileURLSample : public BaseObject {
  friend class FileURLRootSample<PathT>;

 public:
  /**
   * \brief Полный путь к файлу, области памяти
   * */
  PathT GetURL() const { return absolute_path_; }
  std::string GetURLStr() const {
    if constexpr (std::is_same<PathT, fs::path>::value) {
      return absolute_path_.string();
    } else {
      return absolute_path_;
    }
  }
  /**
   * \brief Тип адреса
   * */
  url_t GetURLType() const { return url_type_; }
  /**
   * \brief Путь отмечен как невалидный
   * */
  bool IsInvalidPath() const { return !is_status_aval(status_); }

  /**
   * \brief Добавить к url файла справа путь `dst`
   * \param url Корневая директория типа FileURLSample
   * \param dtr Относительный путь
   * \return Конкатенированную путь - слева url
   * */
  static FileURLSample<PathT> ConcatPathR(const FileURLSample<PathT>& url,
                                          const PathT& dtr) {
    return FileURLSample<PathT>::concat(url.GetURL(), dtr, url.GetURLType());
  }
  /**
   * \brief Добавить к url файла справа путь `dst`
   * \param url Корневая директория
   * \param dtr Относительный путь типа FileURLSample
   * \return Конкатенированную путь - слева url
   * */
  static FileURLSample<PathT> ConcatPathL(const PathT& dtr,
                                          const FileURLSample<PathT>& url) {
    return FileURLSample<PathT>::concat(dtr, url.GetURL(), url.GetURLType());
  }

 private:
  FileURLSample() : BaseObject(STATUS_NOT), url_type_(url_t::empty) {}
  explicit FileURLSample(const PathT& path);
  FileURLSample(url_t url_type, const PathT& path);

  static FileURLSample<PathT> concat(const PathT& c,
                                     const PathT& d,
                                     url_t url_type);

 private:
  /**
   * \brief Тип адреса
   * */
  url_t url_type_;
  /**
   * \brief Полный путь к файлу
   * */
  PathT absolute_path_;
};
using FileURL = FileURLSample<std::string>;

/**
 * \brief Добавить к url файла справа путь `dst`
 * \param url Корневая директория типа FileURLSample
 * \param dtr Относительный путь
 * \return Конкатенированную путь - слева url
 * */
template <PathType PathT>
FileURLSample<PathT> operator+(const FileURLSample<PathT>& url, PathT& dtr) {
  return FileURLSample<PathT>::ConcatPathR(url, dtr);
}
/**
 * \brief Добавить к пути `dst` справа url файла
 * \param url Корневая директория
 * \param dtr Относительный путь типа FileURLSample
 * \return Конкатенированную путь - слева dtr
 * */
template <PathType PathT>
FileURLSample<PathT> operator+(const PathT& dtr,
                               const FileURLSample<PathT>& url) {
  return FileURLSample<PathT>::ConcatPathL(dtr, url);
}

/**
 * \brief Фабрика инициализации файловых адресов
 * */
template <PathType PathT>
class FileURLRootSample : public BaseObject {
 public:
  /**
   * \brief Контейнер содержимого директории
   * */
  struct ContentContainer {
    typedef std::vector<FileURLSample<PathT>> ContainerT;

   public:
    ContainerT content;

   public:
    /**
     * \brief Добавить элемент к контейнеру
     * */
    template <class T>
    void add(T&& element) {
      content.push_back(element);
    }
    /**
     * \brief Container begin
     * */
    template <class T>
    ContainerT::iterator begin() {
      return content.begin();
    }
    /**
     * \brief Container end
     * */
    template <class T>
    ContainerT::iterator end() {
      return content.end();
    }
  };

 public:
  explicit FileURLRootSample(const SetupURLSample<PathT>& setup)
      : BaseObject(STATUS_DEFAULT), setup_(setup) {
    if (setup_.GetURLType() == url_t::fs_path)
      check_fs_root();
  }

  FileURLRootSample(url_t url_type, const PathT& root)
      : BaseObject(STATUS_DEFAULT), setup_(url_type, root) {
    if (setup_.GetURLType() == url_t::fs_path)
      check_fs_root();
  }

  /**
   * \brief Инициализация прошла успешно
   * */
  inline bool IsInitialized() { return is_status_ok(status_); }
  /**
   * \brief Получить адрес корневой директории
   * \return Объект FileURL указывающий на корневую директорию
   * */
  FileURLSample<PathT> GetRootURL() const {
    return FileURLSample<PathT>(setup_.GetURLType(), setup_.GetFullPrefix());
  }
  /**
   * \brief Создать NullObject FileURL
   * \return NullObject FileURL
   * */
  FileURLSample<PathT> CreateNullObjectURL() const;
  /**
   * \brief Собрать адрес файла по относительному пути
   * \param relative_path Относительный путь
   * \return Объект FileURL указывающий на relative_path
   *   относительно корневой директории
   * */
  FileURLSample<PathT> CreateFileURL(const PathT& relative_path);
  /**
   * \brief Список файлов в директории
   * \todo можно добавить относительный путь и глубину обхода(сейчас 1)
   * */
  // ContentContainer GetContent();

 private:
  /**
   * \brief Проверить root директорию файловой системы
   * */
  void check_fs_root();
  /**
   * \brief Собрать адрес файла для случая файловой системы
   * \param Относительный путь к файлу в файловой системе
   * */
  FileURLSample<PathT> set_fs_path(const PathT& relative_path);
  /**
   * \brief Проверить что путь относительный, а не абсолютный
   * \param Проверяемый путь
   * \return true если путь абсолютный
   * */
  bool is_absolute_path(const PathT& path);

 private:
  SetupURLSample<PathT> setup_;
};
using FileURLRoot = FileURLRootSample<std::string>;

template <PathType PathT>
FileURLSample<PathT> FileURLRootSample<PathT>::CreateNullObjectURL() const {
  FileURLSample<PathT> f(setup_.GetURLType(), setup_.GetFullPrefix());
  f.status_ = STATUS_NOT;
  return f;
}

template <PathType PathT>
FileURLSample<PathT> FileURLRootSample<PathT>::CreateFileURL(
    const PathT& relative_path) {
  if (is_status_ok(status_)) {
    switch (setup_.GetURLType()) {
      case url_t::fs_path:
        return set_fs_path(relative_path);
      case url_t::empty:
        break;
    }
  }
  return FileURLSample<PathT>();
}

template <PathType PathT>
void FileURLRootSample<PathT>::check_fs_root() {
  status_ = is_exists(setup_.root) ? STATUS_OK : STATUS_NOT;
}

template <PathType PathT>
FileURLSample<PathT> FileURLRootSample<PathT>::set_fs_path(
    const PathT& relative_path) {
  const auto concat_paths = [](auto&& prefix, const auto& relative) {
    if constexpr (std::is_same<PathT, fs::path>::value)
      return prefix / relative;
    else if constexpr (std::is_same<PathT, std::string>::value)
      return (fs::path(prefix) / fs::path(relative)).string();
  };
  return FileURLSample<PathT>(
      setup_.GetURLType(),
      (is_absolute_path(relative_path))
          ? relative_path
          : concat_paths(setup_.GetFullPrefix(), relative_path));
}

template <PathType PathT>
bool FileURLRootSample<PathT>::is_absolute_path(const PathT& path) {
  if constexpr (std::is_same<PathT, fs::path>::value)
    return path.is_absolute();
  return fs::path(path).is_absolute();
}

// FileURLSample
template <PathType PathT>
FileURLSample<PathT>::FileURLSample(const PathT& path) : FileURLSample() {
  if constexpr (std::is_same<PathT, fs::path>::value) {
    url_type_ = url_t::fs_path;
  } else {
    throw std::exception(
        "Невозможно определить тип `url_type` для "
        "FileURLSample. Используйте явнный конструктор");
  }
  absolute_path_(path);
  if (is_exists(absolute_path_))
    status_ = STATUS_OK;
}
template <PathType PathT>
FileURLSample<PathT>::FileURLSample(url_t url_type, const PathT& path)
    : BaseObject(STATUS_DEFAULT), url_type_(url_type), absolute_path_(path) {
  if (is_exists(absolute_path_))
    status_ = STATUS_OK;
}
template <PathType PathT>
FileURLSample<PathT> FileURLSample<PathT>::concat(const PathT& c,
                                                  const PathT& d,
                                                  url_t url_type) {
  if constexpr (std::is_same<PathT, fs::path>::value) {
    return FileURLSample<PathT>(url_type, c / d);
  } else if (std::is_same<PathT, std::string>::value) {
    return FileURLSample<PathT>(url_type, c + path_separator + d);
  }
  throw std::exception();
  return FileURLSample<PathT>();
}
}  // namespace file_utils
}  // namespace asp_utils

#endif  // !UTILS__FILEURL_H
