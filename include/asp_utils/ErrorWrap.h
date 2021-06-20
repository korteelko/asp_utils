/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__ERRORWRAP_H
#define UTILS__ERRORWRAP_H

#include "asp_utils/Common.h"
#include "asp_utils/ThreadWrap.h"

#include <string>

#include <stdint.h>

#if defined(_DEBUG)
#define STRING_DEBUG_INFO                                \
  ("file: " + std::string(__FILE__) + "\n\tfunction: " + \
   std::string(__FUNCTION__) + " line: " + std::to_string(__LINE__) + "\n")
#else
#define STRING_DEBUG_INFO std::string("")
#endif  // _DEBUG

/**
 * \brief Составить стандартную пару (код ошибки, соответствующее сообщение)
 * */
#define ERROR_PAIR_DEFAULT(x) x, x##_MSG
#define ERROR_SUCCESS_T 0x0000

#define ERROR_SUCCESS_T_MSG "there are not any errors "
#define ERROR_GENERAL_T 0x0001
#define ERROR_GENERAL_T_MSG "general error "

/**
 * \brief Ошибка файлового ввода/вывода
 * */
#define ERROR_FILEIO_T 0x0002
#define ERROR_FILEIO_T_MSG "fileio error "
/**
 * \brief Ошибка файлового ввода/вывода
 * */
#define ERROR_PARSER_T 0x0003
#define ERROR_PARSER_T_MSG "parser error "
/**
 * \brief Ошибка при работе со строками
 * */
#define ERROR_STRING_T 0x0004
#define ERROR_STRING_T_MSG "string processing error "
/**
 * \brief Ошибка инициализации
 * */
#define ERROR_INIT_T 0x0005
#define ERROR_INIT_T_MSG "init struct error "
/**
 * \brief Ошибка типов
 * */
#define ERROR_TYPES_T 0x0006
#define ERROR_TYPES_T_MSG "types error "

//   fileio errors
/// Ошибка чтения файла
#define ERROR_FILE_IN_ST (0x0100 | ERROR_FILEIO_T)
#define ERROR_FILE_IN_ST_MSG "input from file error "
/// Ошибка записи в файл
#define ERROR_FILE_OUT_ST (0x0200 | ERROR_FILEIO_T)
#define ERROR_FILE_OUT_ST_MSG "output to file error "
/// Ошибка операции с файлом логирования
#define ERROR_FILE_LOGGING_ST (0x0300 | ERROR_FILEIO_T)
#define ERROR_FILE_LOGGING_ST_MSG "error with logging file "
/// Ошибка существования файла
#define ERROR_FILE_EXISTS_ST (0x0400 | ERROR_FILEIO_T)
#define ERROR_FILE_EXISTS_ST_MSG "parse json error "

//   parser errors
/// Ошибка парсинга файла
#define ERROR_PARSER_PARSE_ST (0x0100 | ERROR_PARSER_T)
#define ERROR_PARSER_PARSE_ST_MSG "parse error "
/// Ошибка несоответствия форматов в файле
#define ERROR_PARSER_FORMAT_ST (0x0200 | ERROR_PARSER_T)
#define ERROR_PARSER_FORMAT_ST_MSG "parser format error "
/// Ошибка обхода дочерних узлов
#define ERROR_PARSER_CHILD_NODE_ST (0x0300 | ERROR_PARSER_T)
#define ERROR_PARSER_CHILD_NODE_ST_MSG "child node error "

//   string errors
#define ERROR_STR_MAX_LEN_ST (0x0100 | ERROR_STRING_T)
#define ERROR_STR_MAX_LEN_ST_MSG "string len error "
#define ERROR_STR_PARSE_ST (0x0200 | ERROR_STRING_T)
#define ERROR_STR_PARSE_ST_MSG "string parsing error "
#define ERROR_STR_NULL_ST (0x0300 | ERROR_STRING_T)
#define ERROR_STR_NULL_ST_MSG "passed null string "
#define ERROR_STR_TOINT_ST (0x0400 | ERROR_STRING_T)
#define ERROR_STR_TOINT_ST_MSG "convert to int "

//   init errors
/// Невалидные значения для инициализации
#define ERROR_INIT_ZERO_ST (0x0100 | ERROR_INIT_T)
#define ERROR_INIT_ZERO_ST_MSG "zero value init "
/// NULL значения при инициализации
#define ERROR_INIT_NULLP_ST (0x0200 | ERROR_INIT_T)
#define ERROR_INIT_NULLP_ST_MSG "nullptr value init "

// types errors
#define ERROR_TYPES_DYNAMIC_ST (0x0100 | ERROR_TYPES_T)
#define ERROR_TYPES_DYNAMIC_ST_MSG "dynamic type error "

namespace asp_utils {
/**
 * \brief Тип кода ошибки
 */
typedef uint32_t merror_t;

/**
 * \brief Класс, в котором инкапсулирована ошибка(код, сообщение,
 *   логирована ли, выведелена ли и т.п.)
 * */
class ErrorWrap {
 public:
  ErrorWrap();
  explicit ErrorWrap(merror_t error);
  ErrorWrap(merror_t error, const std::string& msg);
  ErrorWrap(merror_t error, std::string&& msg);
  /**
   * \brief Установить(хранить) код ошибки 'error'
   * \param Error код ошибки
   * \param Msg сопроводительное сообщение
   * */
  merror_t SetError(merror_t error, const std::string& msg);
  /**
   * \brief Заменить сообщение об ошибке 'msg_' на 'msg'
   * \param msg
   * */
  void SetErrorMessage(const std::string& msg);
  /**
   * \brief Залогировать текущее состояние
   * \param lvl Уровень логирования данной ошибки
   *
   * Если уровень логирования состояния программы соответствует
   * переданному уровню логирования в аргументах и, собственно говоря,
   * если есть что логировать.
   * Установить 'is_logged_' в true
   * */
  void LogIt(io_loglvl lvl = io_loglvl::err_logs);
  /**
   * \brief Залогировать текущее состояние
   * \param pl Ссылка на объект логирования
   * \param lvl Уровень логирования данной ошибки
   *
   * Если уровень логирования состояния программы соответствует
   * переданному уровню логирования в аргументах и, собственно говоря,
   * если есть что логировать.
   * Установить 'is_logged_' в true
   * */
  void LogIt(class PrivateLogging& pl, io_loglvl lvl = io_loglvl::err_logs) const;
  void LogIt(class PrivateLogging& pl,
             const std::string& logger,
             io_loglvl lvl = io_loglvl::err_logs) const;
  /**
   * \brief Получить код ошибки
   * */
  merror_t GetErrorCode() const;
  /**
   * \brief Получить копию сообщения об ошибке
   * */
  std::string GetMessage() const;
  /**
   * \brief Создать сообщение об ошибке включающее оригинальное
   *   сообщение и код ошибки
   * */
  std::string CreateErrorMessage() const;
  /**
   * \brief Флаг того что сообщение об ошибке уже залогировано
   * */
  bool IsLogged() const;
  /**
   * \brief Скинуть все параметры объекта ErrorWrap в значения по умолчанию
   *
   * Параметры по умрлчанию
   *   error_ to ERROR_SUCCESS_T
   *   msg_ to ""
   *   is_logged_ to false
   * */
  void Reset();

 private:
  /**
   * \brief Код ошибки(см merror_codes.h)
   * */
  merror_t error_;
  /**
   * \brief Сообщение к ошибке
   * */
  std::string msg_;
  /**
   * \brief Мьютекс на обновление данных, на логирование
   * */
  Mutex update_mutex_;
  /**
   * \brief Переменная отслеживающая выводилось ли уже
   *   информация об этой ошибке
   * \note Скидываем на false при обновлении ошибки(SetError)
   * */
  bool is_logged_;
};
}  // namespace asp_utils

#endif  // !UTILS__ERRORWRAP_H
