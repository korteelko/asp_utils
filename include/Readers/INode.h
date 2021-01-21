/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__INODE_H
#define UTILS__INODE_H

#include "Common.h"

#include <string>
#include <vector>

#include <stdint.h>


template <class Initializer>
class SimpleInitializerFactory {
public:
  Initializer *GetNodeInitializer() { return new Initializer(); }
};

// typedef int32_t node_id;
/** \brief вектор имён дочерних элементов */
typedef std::vector<std::string> inodes_vec;

/* todo: можно бы шаблонный параметр добавить */
/** \brief Интерфейс узла для составления параметров шаблона
  *   классов JSONReader и XMLReader
  * \note По идее обвязка над структурой(ами) которые нужно
  *   инициализировать парсером  */
class INodeInitializer {
public:
  INodeInitializer() = default;
  virtual ~INodeInitializer() = default;

  // node_id GetId() const { return id_; }
  /* maybe virtual... ??? */
  std::string GetName() const { return name_; }
  mstatus_t GetStatus() const { return status_; }

  /** \brief Узел является простым - не содержит подузлов
    *   и вложенных параметров */
  virtual bool IsLeafNode() const { return subnodes_.empty(); }
  /** \brief Получить параметр по имени */
  virtual std::string GetParameter(const std::string &name) = 0;
  /** \brief Записать имена узлов, являющихся
    *   контейнерами других объектов */
  virtual void SetSubnodesNames(inodes_vec *subnodes) = 0;

  /** \brief Инициализировать данные родительского узла */
  void SetParentData(INodeInitializer &parent) { (void) parent; }

protected:
  mstatus_t status_ = STATUS_DEFAULT;
  /** \brief уникальный идентификатор ноды */
  // node_id id_;
  /** \brief собственное имя ноды */
  std::string name_;
  /** \brief вектор имён поднод(подузлов)
    * \note по ним из класса парсера их можно инициализировать */
  inodes_vec subnodes_;
};

#endif  // !UTILS__INODE_H
