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

#include "Common.h"
#include "ErrorWrap.h"

#include <string>
#include <vector>

namespace file_utils {
/* todo: create class file_utils exception */
class FileURLRoot;

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
 * \brief Сетап URL - хост, юзер и т.п., все входные
 *   данные для фабрики URL объектов короч
 * \note Сейчас то это просто для красоты
 * \todo replace 'root' type with struct wth overloaded operator[]
 *   for c++17 comp
 * */
struct SetupURL {
 public:
  SetupURL(url_t url_type, const std::string& root);
  /**
   * \brief Полный префикс для пути(от начала до рута)
   * */
  std::string GetFullPrefix() const;
  /**
   * \brief Тип ссылки
   * */
  url_t GetURLType() const;

 public:
  /**
   * \brief Тип адресов для создания полных путей
   * */
  url_t url_type;
  /**
   * \brief Путь к корню системы(для файловой системы)
   * */
  std::string root;
};
inline std::string SetupURL::GetFullPrefix() const {
  return root;
}
inline url_t SetupURL::GetURLType() const {
  return url_type;
}

/**
 * \brief Класс пути(м.б. обычный пути в файловой системе, урл,
 *   запрос к дб и т.п.)
 * \note Немножко ликбеза:
 *   вид File(URI):
 *     "file://<host>/<path>"
 *   А вот обощённый URL несёт много больше информации:
 *     "<схема>:[//[<логин>[:<пароль>]@]<хост>[:<порт>]][/<URL‐путь>][?<параметры>][#<якорь>]"
 *   Естественно, полный url нам пока не нужен, но, лучше ввести лишнюю
 *     структуру хранящую все параметры подключения(хост, юзер с поролем),
 *     которую будем хранить в фабрике
 * */
class FileURL {
  friend class FileURLRoot;

 public:
  /**
   * \brief Полный путь к файлу, области памяти
   * */
  std::string GetURL() const;
  /**
   * \brief Получить код ошибки
   * */
  merror_t GetError() const;
  /**
   * \brief Получить состояние объекта
   * */
  mstatus_t GetStatus() const;
  /**
   * \brief Путь отмечен как невалидный
   * */
  bool IsInvalidPath() const;
  /**
   * \brief Установить код ошибки 'error' и сообщение ошибки 'msg'
   * */
  void SetError(merror_t error, const std::string& msg);
  /**
   * \brief Залогировать ошибки
   * */
  void LogError();

 private:
  FileURL();
  FileURL(url_t url_type, const std::string& path);

 private:
  ErrorWrap error_;
  mstatus_t status_;
  /**
   * \brief Тип адреса
   * */
  url_t url_type_;
  /**
   * \brief Полный путь к файлу
   * */
  std::string absolute_path_;
};
inline std::string FileURL::GetURL() const {
  return absolute_path_;
}
inline merror_t FileURL::GetError() const {
  return error_.GetErrorCode();
}
inline mstatus_t FileURL::GetStatus() const {
  return status_;
}
/**
 * \brief Добавить к url файла справа строку `dst`
 * \return Конкатенированную строку - слева url
 * */
std::string operator+(const FileURL& url, const std::string& dtr);
/**
 * \brief Добавить к строке `dst` справа url файла
 * \return Конкатенированную строку - слева dtr
 * */
std::string operator+(const std::string& dtr, const FileURL& url);

/**
 * \brief Фабрика инициализации файловых адресов
 * */
class FileURLRoot {
 public:
  /**
   * \brief Контейнер содержимого директории
   * */
  struct ContentContainer {
    typedef std::vector<FileURL> ContainerT;

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
  explicit FileURLRoot(const SetupURL& setup);
  FileURLRoot(url_t url_type, const std::string& root);
  /**
   * \brief Инициализация прошла успешно
   * */
  bool IsInitialized();
  /**
   * \brief Получить адрес корневой директории
   * \return Объект FileURL указывающий на корневую директорию
   * */
  FileURL GetRootURL() const;
  /**
   * \brief Создать NullObject FileURL
   * \return NullObject FileURL
   * */
  FileURL CreateNullObjectURL() const;
  /**
   * \brief Собрать адрес файла по относительному пути
   * \param relative_path Относительный путь
   * \return Объект FileURL указывающий на relative_path
   *   относительно корневой директории
   * */
  FileURL CreateFileURL(const std::string& relative_path);
  /**
   * \brief Список файлов в директории
   * \todo можно добавить относительный путь и глубину обхода(сейчас 1)
   * */
  ContentContainer GetContent();

 private:
  /**
   * \brief Проверить root директорию файловой системы
   * */
  void check_fs_root();
  /**
   * \brief Собрать адрес файла для случая файловой системы
   * */
  FileURL set_fs_path(const std::string& relative_path);
  /**
   * \brief Проверить что путь относительный, а не абсолютный
   * */
  bool is_absolute_path(const std::string& path);

 private:
  mstatus_t status_;
  SetupURL setup_;
};
}  // namespace file_utils

#endif  // !UTILS__FILEURL_H
