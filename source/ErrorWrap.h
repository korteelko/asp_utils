/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__ERRORWRAP_H
#define UTILS__ERRORWRAP_H

#include "Common.h"
#include "ThreadWrap.h"
/**
 * \note Для разных проектов разные коды ошибок,
 *   поэтому дефайны вынесены в отдельный файл
 *
 * \todo Какое-то плохое решение, больше так не делать
 * */
#if defined(INCLUDE_ERRORCODES)
#include "merror_codes.h"
#endif  // INCLUDE_ERRORCODES

#include <string>

#include <stdint.h>

#if defined(_DEBUG)
#define STRING_DEBUG_INFO                                \
  ("file: " + std::string(__FILE__) + "\n\tfunction: " + \
   std::string(__FUNCTION__) + " line: " + std::to_string(__LINE__) + "\n")
#else
#define STRING_DEBUG_INFO std::string("")
#endif  // _DEBUG

typedef uint32_t merror_t;

#if not defined(ERROR_SUCCESS_T)
#define ERROR_SUCCESS_T 0x0000
#define ERROR_SUCCESS_T_MSG "there are not any errors "
#endif  // !ERROR_SUCCESS_T
#if not defined(ERROR_GENERAL_T)
#define ERROR_GENERAL_T 0x0001
#define ERROR_GENERAL_T_MSG "general error "
#endif  // !ERROR_GENERAL_T

/** \brief Подтип ошибок для модулей */
#define ERROR_OTHER_MODULE_T 0x000f

/** \brief Составить стандартную пару (код ошибки, соответствующее сообщение) */
#define ERROR_PAIR_DEFAULT(x) x, x##_MSG

/** \brief ошибка файлового ввода/вывода */
#define ERROR_FILEIO_T 0x0002
#define ERROR_FILEIO_T_MSG "fileio error "
/** \brief ошибка при работе со строками */
#define ERROR_STRING_T 0x0004
#define ERROR_STRING_T_MSG "string processing error "
/** \brief ошибка инициализации */
#define ERROR_INIT_T 0x0005
#define ERROR_INIT_T_MSG "init struct error "
//   fileio errors
/** \brief ошибка чтения файла */
#define ERROR_FILE_IN_ST (0x0100 | ERROR_FILEIO_T)
#define ERROR_FILE_IN_ST_MSG "input from file error "
/** \brief ошибка записи в файл */
#define ERROR_FILE_OUT_ST (0x0200 | ERROR_FILEIO_T)
#define ERROR_FILE_OUT_ST_MSG "output to file error "
/** \brief ошибка операции с файлом логирования */
#define ERROR_FILE_LOGGING_ST (0x0300 | ERROR_FILEIO_T)
#define ERROR_FILE_LOGGING_ST_MSG "error with logging file "
/** \brief ошибка существования файла */
#define ERROR_FILE_EXISTS_ST (0x0400 | ERROR_FILEIO_T)
#define ERROR_FILE_EXISTS_ST_MSG "parse json error "

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
#define ERROR_INIT_ZERO_ST (0x0100 | ERROR_INIT_T)
#define ERROR_INIT_ZERO_ST_MSG "zero value init "
#define ERROR_INIT_NULLP_ST (0x0200 | ERROR_INIT_T)
#define ERROR_INIT_NULLP_ST_MSG "nullptr value init "

/**
 * \brief класс, в котором инкапсулирована ошибка(код, сообщение,
 *   логирована ли, выведелена ли и т.п.)
 * */
class ErrorWrap {
 public:
  ErrorWrap();
  explicit ErrorWrap(merror_t error);
  ErrorWrap(merror_t error, const std::string& msg);
  /**
   * \brief Установить(хранить) код ошибки 'error'
   * \param Error код ошибки
   * \param Msg сопроводительное сообщение
   * */
  merror_t SetError(merror_t error, const std::string& msg);
  /**
   * \brief Заменить сообщение об ошибке 'msg_' на 'msg'
   * */
  void SetErrorMessage(const std::string& msg);
  /**
   * \brief Залогировать текущее состояние, если есть ошибка.
   *   Установить 'is_logged_' в true
   * \param lvl(optional) Особый логлевел для данного сообщения
   * \note Здесь и везде Log* методы не константные,
   *   т.к. в них отслеживается состояние
   * */
  void LogIt();
  /**
   * \brief Залогировать текущее состояние, если уровень логирования
   *   состояния программы соответствует переданному уровню логирования
   *   в аргументах и, собственно говоря, если есть что логировать.
   *   Установить 'is_logged_' в true
   *
   * \param lvl Уровень логирования данной ошибки
   * */
  void LogIt(io_loglvl lvl);
  /**
   * \brief Получить код ошибки
   * */
  merror_t GetErrorCode() const;
  /**
   * \brief Получить копию сообщения об ошибке
   * */
  std::string GetMessage() const;
  /**
   * \brief Получить указатель на данные сообщения.
   *
   * \warning Небезопасный метод, если объект будет уничтожен, а указатель
   *   ещё где-то висеть будет, неприятная ошибка прокинется, и наверное
   *   неизвестно где.
   *   Лучше, конечно, пользовать GetMessage
   * */
  const char* GetMessagePtr() const noexcept;
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

  /* DEVELOP: такая перегрузка может стать причиной ментального бо-бо */
  ErrorWrap& operator=(merror_t) = delete;

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

#endif  // !UTILS__ERRORWRAP_H
