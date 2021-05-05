/**
 * asp_utils library
 * ===================================================================
 * * Base *
 * Базовые классы библиотеки asp_utils
 * ===================================================================
 *
 * Copyright (c) 2020-2021
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _UTILS__BASE_H_
#define _UTILS__BASE_H_

#include "asp_utils/Common.h"
#include "asp_utils/ErrorWrap.h"

namespace asp_utils {
/**
 * \brief Базовый объект библиотеки asp_utils имплементирующий функционал
 *   статуса и ошибок
 */
struct BaseObject {
 public:
  explicit BaseObject(mstatus_t status) : status_(status) {}
  virtual ~BaseObject() = default;
  /**
   * \brief Получить код ошибки
   * */
  inline merror_t GetError() const { return error_.GetErrorCode(); }
  /**
   * \brief Получить состояние объекта
   * */
  inline mstatus_t GetStatus() const { return status_; }
  /**
   * \brief Установить код ошибки 'error' и сообщение ошибки 'msg'
   * */
  inline void SetError(merror_t error, const std::string& msg) {
    error_.SetError(error, msg);
    status_ = STATUS_HAVE_ERROR;
  }
  /**
   * \brief Залогировать ошибки
   * */
  inline void LogError() { error_.LogIt(); }

 protected:
  mstatus_t status_;
  ErrorWrap error_;
};

/**
 * \brief Интерфейс имплементаций паттерна NullObject
 * \note Можно не наследоваться от этого интерфейса, а использовать
 *   агрегирующий класс OptionalSharedPtr
 * */
struct INullObject {
 public:
  virtual ~INullObject() = default;
  /**
   * \brief Перегружаемый метод исполнения
   * */
  virtual void Perform() = 0;
};

/**
 * \brief Класс optional-обёртка над shared_ptr
 * \tparam T Тип данных спрятанный в умный указатель
 *
 * Класс представляет собой имплементацию паттерна null object,
 * с несколько корявой логикой - этот класс обеспечивает
 * интерфейс для объекта агрегатора для null object, а не сам
 * null object интерфейс.
 * */
template <class T>
struct OptionalSharedPtr {
  virtual ~OptionalSharedPtr() = default;

  OptionalSharedPtr() : status_(STATUS_NOT), data_(std::nullopt) {}
  OptionalSharedPtr(const std::shared_ptr<T>& data)
      : status_(STATUS_DEFAULT), data_(data) {}
  template <class... Args>
  OptionalSharedPtr(Args&&... args)
      : status_(STATUS_DEFAULT), data_(std::make_shared<T>(args...)) {}
  /**
   * \brief Перегружаемый метод воспроизведения действия
   *   null object объекта.
   * */
  virtual void perform() { status_ = STATUS_NOT; }
  /**
   * \brief Проверить наличие данных
   *
   * \return true если данные есть
   *         false если значение std::nullopt
   * */
  bool has_value() const { return data_ != std::nullopt; }
  /**
   * \brief Получить указатель на данные
   *
   * \throw std::bad_optional_access
   * \return Умный указатель на данные
   * */
  std::shared_ptr<T> get_data() const { return data_.value(); }

 public:
  /**
   * \brief Состояние объекта
   * */
  mstatus_t status_;
  /**
   * \brief std::optional обёртка над умным указателем на данные
   * */
  std::optional<std::shared_ptr<T>> data_ = std::nullopt;
};

/**
 * \brief OptionalSharedPtr имплементация для интерфейса NullObject
 * */
template <class T,
          std::enable_if_t<std::is_base_of<INullObject, T>::value, bool> = true>
struct NullObjectSharedPtr : public OptionalSharedPtr<T> {
  NullObjectSharedPtr() : OptionalSharedPtr<T>() {}
  NullObjectSharedPtr(const std::shared_ptr<T>& data)
      : OptionalSharedPtr<T>(data) {}
  template <class... Args>
  NullObjectSharedPtr(Args... args) : OptionalSharedPtr<T>(args...) {}
  void perform() override {
    OptionalSharedPtr<T>::status_ = STATUS_NOT;
    if (OptionalSharedPtr<T>::has_value()) {
      OptionalSharedPtr<T>::get_data()->Perform();
      OptionalSharedPtr<T>::status_ = STATUS_OK;
    }
  }
};
}  // namespace asp_utils

#endif  // !_UTILS__BASE_H_
