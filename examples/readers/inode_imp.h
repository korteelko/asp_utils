#ifndef TESTS__UTILS__INODE_IMP_H
#define TESTS__UTILS__INODE_IMP_H

#include "asp_utils/Common.h"
#include "asp_utils/ErrorWrap.h"
#include "asp_utils/Readers/INode.h"
#include "asp_utils/Readers/JSONReader.h"
#include "asp_utils/Readers/XMLReader.h"

#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

using namespace asp_utils;
/*
json:
{
  "test": {
    "first": {
      "f": "sda",
      "s": "sdsa",
      "t": 116.2,
      "ff": ""
    },
    "second": {
      "f": "asd",
      "s": 32,
      "t": 12
    }
  }
}

xml:
<?xml version="1.0" encoding="UTF-8"?>
<test>
  <first>
    <f> sda </f>
    <s> sdsa </s>
    <t> 116.2 </t>
    <ff/>
  </first>
  <second>
    <f> asd </f>
    <s> 32  </s>
    <t> 12  </t>
  </second>
</test>
*/
struct first {
  std::string f, s, ff;
  float t;
};
struct second {
  std::string f;
  int s, t;
};

template <class ReaderNodeT>
class json_test_node;

/** \brief Фабрика создания test_node */
// template <class >
class json_test_factory {
 public:
  std::vector<first> vf;
  std::vector<second> vs;

 public:
  template <class ReaderNodeT>
  json_test_node<ReaderNodeT>* GetNodeInitializer() {
    json_test_node<ReaderNodeT>* node = new json_test_node<ReaderNodeT>(this);
    if (node)
      ++factory_num;
    return node;
  }

 public:
  int factory_num = 0;
};

/** \brief тестовая структура-параметр шаблонов парсеров */
template <class ReaderNodeT>
class json_test_node : public INodeInitializer {
 public:
  ReaderNodeT* source = nullptr;
  std::string parent_name;
  /* also it is container */
  json_test_factory* factory;
  bool have_subnodes_ = false;

 public:
  json_test_node() {}

  json_test_node(json_test_factory* factory) : factory(factory) { name_ = ""; }

  /** \brief Инициализировать данные структуры,
   *   специализация для rapidjson
   * \note зачем здесь nodename */
  merror_t InitData(rj::Value* src, const std::string& nodename) {
    if (!src)
      return ERROR_INIT_NULLP_ST;
    name_ = nodename;
    source = src;
    merror_t error = ERROR_SUCCESS_T;
    set_subnodes();
    if (name_ == "first") {
      factory->vf.push_back(getFirst());
    } else if (name_ == "second") {
      factory->vs.push_back(getSecond());
    }
    if (name_.empty()) {
      error = ERROR_GENERAL_T;  // ERROR_JSON_FORMAT_ST;
    }
    return error;
  }
  /** \brief Инициализировать данные структуры,
   *   специализация для rapidjson */
  merror_t InitData(pugi::xml_node* src, const std::string& nodename) {
    if (!src)
      return ERROR_INIT_NULLP_ST;
    name_ = nodename;
    source = src;
    set_subnodes();
    if (name_ == "first") {
      factory->vf.push_back(getFirst());
    } else if (name_ == "second") {
      factory->vs.push_back(getSecond());
    }
    return ERROR_SUCCESS_T;
  }

  void SetParentData(json_test_node& parent) { parent_name = parent.GetName(); }

  // override
  bool IsLeafNode() const override { return have_subnodes_; }
  /** \brief получить поле узла структуры */
  void SetSubnodesNames(inodes_vec* s) override {
    s->clear();
    s->insert(s->end(), subnodes_.cbegin(), subnodes_.cend());
  }
  /** \brief получить поле узла структуры */
  std::string GetParameter(const std::string& name) override {
    return json_test_node::getParameter(source, name);
  }

 private:
  // static std::string getParameter(ReaderNodeT *src, const std::string &name);
  static std::string getParameter(rjNValue* src, const std::string& name) {
    std::string value = "";
    if (src->HasMember(name.c_str())) {
      rjNValue& par = src->operator[](name.c_str());
      if (par.IsString()) {
        value = par.GetString();
      } else if (par.IsInt()) {
        value = std::to_string(par.GetInt());
      } else if (par.IsDouble()) {
        value = std::to_string(par.GetDouble());
      }
    }
    return value;
  }
  static std::string getParameter(pugi::xml_node* src,
                                  const std::string& name) {
    auto p = src->child(name.c_str());
    std::string p_name = p.name();
    p.print(std::cout);
    return (p.empty()) ? "" : p.first_child().value();
  }
  /** \brief проверить наличие подузлов
   * \note просто посмотрим тип этого узла,
   *   а для случая придумаю что-нибудь */
  void set_subnodes() {
    subnodes_.clear();
    if (name_ == "test") {
      have_subnodes_ = true;
      subnodes_.push_back("first");
      subnodes_.push_back("second");
    }
    if (name_ == "first") {
      have_subnodes_ = true;
    }
    if (name_ == "second") {
      have_subnodes_ = true;
    } else {
      have_subnodes_ = false;
    }
  }
  /* jfl */
  /** \brief инициализировать параметры тестовой структуры first */
  first getFirst() {
    first f;
    f.f = GetParameter("f");
    f.s = GetParameter("s");
    f.ff = GetParameter("ff");
    try {
      f.t = std::stof(GetParameter("t"));
    } catch (std::exception& e) {
      Logging::Append(std::string("Ошибка инициализации float параметра ") +
                      e.what());
    }
    return f;
  }
  /** \brief инициализировать параметры тестовой структуры second */
  second getSecond() {
    second s;
    s.f = GetParameter("f");
    try {
      s.s = std::atoi(GetParameter("s").c_str());
      s.t = std::atoi(GetParameter("t").c_str());
    } catch (std::exception& e) {
      Logging::Append(std::string("Ошибка инициализации float параметра ") +
                      e.what());
    }
    return s;
  }
};

#endif  // !TESTS__UTILS__INODE_IMP_H
