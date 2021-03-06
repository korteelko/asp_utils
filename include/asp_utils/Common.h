/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * Common *
 *   Общий функционал программы - инициализация прокинутых с CMake
 * дефайнов и т.п.
 * ===================================================================
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _UTILS__COMMON_H_
#define _UTILS__COMMON_H_

#include <algorithm>
#include <complex>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#if defined(BYCMAKE_DEBUG)
/** \brief Режим отладки */
#define _DEBUG
#endif  // BYCMAKE_DEBUG
#if defined(BYCMAKE_WITH_POSTGRESQL)
/** \brief Использовать библиотеку libpqxx */
#define WITH_POSTGRESQL
#endif  // BYCMAKE_WITH_POSTGRESQL
#if defined(BYCMAKE_WITH_PUGIXML)
#define WITH_PUGIXML
#endif  // BYCMAKE_WITH_PUGIXML
#if defined(BYCMAKE_WITH_RAPIDJSON)
#define WITH_RAPIDJSON
#endif  // BYCMAKE_WITH_RAPIDJSON
#if defined(BYCMAKE_TESTS_ENABLED)
#define TESTS_ENABLED
#endif  // BYCMAKE_TESTS_ENABLED

#if defined(OS_WINDOWS)
#define path_separator '\\'
#elif defined(OS_UNIX)
#define path_separator '/'
#endif  // OS_

// кажется такой подход несколько ломает логику
#if defined(TESTING_PROJECT)
#define ADD_TEST_CLASS(x) friend class x;
#else
#define ADD_TEST_CLASS(x)
#endif  // TEST_PROJECT

//  math defines
#define FLOAT_ACCURACY 0.00001
#define DOUBLE_ACCURACY 0.000000001

// status/state defines
/** \brief статус при инициализации */
#define STATUS_DEFAULT 0x00000001
/** \brief статус удачного результата операции */
#define STATUS_OK 0x00000002
/** \brief статус неудачного результата операции */
#define STATUS_NOT 0x00000003
/** \brief статус наличия ошибки при выполнении операции */
#define STATUS_HAVE_ERROR 0x00000004

// logging defines
/**
 * \brief Уровень логирования релиз сборки
 * */
#define DEFAULT_LOGLVL 0x01
/**
 * \brief Уровень логирования отладочной сборки
 * */
#define DEBUG_LOGLVL 0x0f

namespace asp_utils {
namespace fs = std::filesystem;
class ErrorWrap;
typedef uint32_t mstatus_t;

/**
 * \brief Уровни логирования
 *
 * \todo Add 'trace' loglevel
 * */
typedef enum {
  /**
   * \brief Не логировать
   * */
  no_log = 0,
  /**
   * \brief Логировать только ошибки
   * */
  err_logs = DEFAULT_LOGLVL,
  /**
   * \brief Предупреждения и ошибки
   * */
  warn_logs,
  /**
   * \brief Логировать все сообщения кроме отладочных
   * */
  info_logs,
  /**
   * \brief Вывод всех сообщений и отладочной информации
   * */
  debug_logs = DEBUG_LOGLVL
  // trace
} io_loglvl;
/**
 * \brief Уровень логирования к строковому представлению
 * */
std::string io_loglvl_str(io_loglvl ll);
/**
 * \brief Вывести целочисленное значение в шестнадцеричном формате
 * \param hex Приводимое к шестнадцетеричному представлению число
 * \return Строка-представление
 * */
template <typename Integer,
          typename = std::enable_if_t<std::is_integral<Integer>::value>>
std::string hex2str(Integer hex) {
  std::stringstream hex_stream;
  hex_stream << "0x" << std::hex << hex;
  return hex_stream.str();
}
/**
 * \brief Проверить допустимость текущего состояния статуса
 * \return true если st == STATUS_DEFAULT или st == STATUS_OK
 * */
inline bool is_status_aval(mstatus_t status) {
  return status == STATUS_DEFAULT || status == STATUS_OK;
}
/**
 * \brief Проверить валидность статуса
 * \return true если st == STATUS_OK
 * */
inline bool is_status_ok(mstatus_t status) {
  return status == STATUS_OK;
}
/**
 * \brief Проверить валидность статуса для набора аргументов
 * \return true если для всех аргументов is_status_ok = true
 * */
template <class... Targs>
bool is_status_ok(mstatus_t status, Targs... fargs) {
  return is_status_ok(status) && is_status_ok(fargs...);
}
/**
 * \brief Строка str заканчивается подстрокой ending
 * \brief true Если str заканчивается подстрокой ending
 * */
inline bool ends_with(const std::string& str, const std::string& ending) {
  if (ending.size() > str.size())
    return false;
  return std::equal(ending.rbegin(), ending.rend(), str.rbegin());
}
/**
 * \brief Обрезать пробелы с обоих концов
 * \return Строка без начальных и конечных пробелов
 * */
std::string trim_str(const std::string& str);
/**
 * \brief Проверить что объект файловой системы(файл, директория, соккет,
 *   линк, character_dev) существует
 * \return true если путь path валиден
 * */
bool is_exists(const std::string& path);
bool is_exists(const fs::path& path);
/**
 * \brief Вернуть путь к директории содержащей файл
 * \return Путь к директории
 * */
std::string dir_by_path(const std::string& path);
/**
 * \brief Разбить строку по сепаратору 'ch' записать изменения
 *   в контейнер
 * \param str Исходная строка
 * \param cont_p Указатель на контейнер полученных строк
 * \param sep Разделитель
 * */
template <class ContainerT>
void split_str(const std::string& str, ContainerT* cont_p, char sep) {
  size_t pos = str.find(sep);
  size_t start = 0;
  while (pos != std::string::npos) {
    cont_p->push_back(str.substr(start, pos - start));
    start = pos + 1;
    pos = str.find(sep, start);
  }
  cont_p->push_back(str.substr(start, std::min(pos, str.size()) - start + 1));
}
/**
 * \brief Слить контейнер данных в строковый поток
 * \param cont Ссылка на контейнер данных
 * \param sep Разделитель значений
 *
 * \return Строковый поток значений контейнера, разбитых `sep`
 *
 * \todo stringstream принимать или не надо - создавать новый?
 *   Может вообще обобщённую функцию создать с возвратом decltype
 *   или вообще в строку лупить
 * */
template <class ContainerT, class SeparatorT>
std::stringstream join_container(const ContainerT& cont, SeparatorT&& sep) {
  std::stringstream sstr;
  if (cont.size()) {
    sstr << cont[0];
    std::for_each(std::next(cont.begin()), cont.end(),
                  [&sstr, &sep](const auto& s) { sstr << sep << s; });
  }
  return sstr;
}
bool is_equal(double a, double b, double accur = FLOAT_ACCURACY);
bool is_equal(std::complex<double> a,
              std::complex<double> b,
              double accur = FLOAT_ACCURACY);
/**
 * \brief Считать файл в кодировке utf8 в строку
 *
 * \param path Путь к файлу
 * \param error Ссылка на объект-ошибку
 * \return std::stringstream Поток строк
 * */
std::stringstream read_file(const fs::path& path, ErrorWrap& error);
}  // namespace asp_utils

#endif  // !_UTILS__COMMON_H_
