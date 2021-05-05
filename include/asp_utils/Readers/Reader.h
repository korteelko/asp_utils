/**
 * utils
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__READER_H
#define UTILS__READER_H

#include "asp_utils/Base.h"
#include "asp_utils/Common.h"
#include "asp_utils/ErrorWrap.h"
#include "asp_utils/FileURL.h"
#include "asp_utils/Logging.h"
#include "asp_utils/Readers/INode.h"
#ifdef WITH_PUGIXML
#include "pugixml.hpp"
#endif  // WITH_PUGIXML

#ifdef WITH_RAPIDJSON
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#endif  // WITH_RAPIDJSON

#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <string.h>

namespace asp_utils {

/**
 * \brief Представление узла древовидной структуры(json или xml)
 *   в той или иной имплементирующей библиотеке
 * \note Можно наверное здесь exceptions попрокидывать
 * */
template <class NodeT>
struct lib_node {
  using NodeDocType = void;

 public:
  NodeT* GetNodePointer() { return nullptr; }
  NodeT* GetChild(const char*) { return nullptr; }
  std::string GetName() { return ""; }
  static bool IsInitialized(const NodeT&) { return false; }
  /**
   * \brief Инициализировать root узел
   * \param NodeDocType * Указатель на инициализируемый документ
   * \param char * Считанный буффер памяти
   * \param size_t Длина считанного буффера памяти
   * \param std::string * out-параметр - имя корневого узла
   * \param ErrorWrap * указатель на объект состояния ошибки
   **/
  static NodeT InitDocumentRoot(NodeDocType*,
                                char*,
                                size_t,
                                std::string*,
                                ErrorWrap*) {
    return NodeT();
  }
};
#ifdef WITH_PUGIXML
/** \brief Представление узла xml в pugi */
template <>
struct lib_node<pugi::xml_node> {
  using NodeDocType = pugi::xml_document;

 public:
  lib_node() {}
  lib_node(pugi::xml_node xn) : data(xn) {}
  lib_node(pugi::xml_node&& xn) : data(xn) {}

  inline pugi::xml_node* GetNodePointer() {
    return (data.empty()) ? nullptr : &data;
  }

  pugi::xml_node GetChild(const char* name) { return data.child(name); }

  static bool IsInitialized(const pugi::xml_node& xn) { return !xn.empty(); }

  static pugi::xml_node InitDocumentRoot(pugi::xml_document* doc,
                                         char* memory,
                                         size_t len,
                                         std::string* root_name,
                                         ErrorWrap* ew) {
    pugi::xml_parse_result res = doc->load_buffer_inplace(memory, len);
    if (!res) {
      ew->SetError(ERROR_PARSER_FORMAT_ST,
                   std::string("pugixml parse error: ") + res.description());
    } else {
      *root_name = doc->begin()->name();
      return *doc->begin();
    }
    return pugi::xml_node();
  }

 public:
  pugi::xml_node data;
};
#endif  // WITH_PUGIXML

#ifdef WITH_RAPIDJSON
namespace rj = rapidjson;
/** \brief Представление узла json в rapudjson */
template <>
struct lib_node<rjNValue> {
  using NodeDocType = rjNDocument;

 public:
  lib_node() {}
  lib_node(rjNValue* jn) : data(jn) {}

  inline rjNValue* GetNodePointer() { return data; }

  rjNValue* GetChild(const char* name) {
    auto ch = data->FindMember(name);
    return (ch != data->MemberEnd()) ? &ch->value : nullptr;
  }

  static bool IsInitialized(const rjNValue* xn) { return xn != nullptr; }

  static rjNValue* InitDocumentRoot(rjNDocument* doc,
                                    char* memory,
                                    size_t,
                                    std::string* root_name,
                                    ErrorWrap* ew) {
    doc->Parse(memory);
    if (!doc->HasParseError()) {
      if (doc->IsObject()) {
        auto root = doc->MemberBegin();
        if (root != doc->MemberEnd()) {
          *root_name = root->name.GetString();
          return &root->value;
        }
      } else {
        ew->SetError(ERROR_PARSER_PARSE_ST,
                     "ошибка инициализации "
                     "корневого элемента json файла ");
      }
    }
    ew->SetError(ERROR_PARSER_FORMAT_ST,
                 std::string("RapidJSON parse error: ") +
                     std::string(rj::GetParseError_En(doc->GetParseError())));
    return nullptr;
  }

 public:
  rjNValue* data = nullptr;
};
#endif  // WITH_RAPIDJSON

// class node_sample
/** \brief Шаблон класса дерева файла, стандартная обёртка
 *   над инициализируемой нодой
 * \note Некоторые функции допустимых фаблонов вынесены в шаблоны
 *   lib_node, хотя их можно было реализовать в классе явной
 *   специализацией функций */
template <class NodeT,
          class Initializer,
          class InitializerFactory,
          class = typename std::enable_if<
              std::is_base_of<INodeInitializer, Initializer>::value>::type>
class node_sample : public BaseObject {
  typedef node_sample<NodeT, Initializer, InitializerFactory> node;
  /** \brief умный указатель на имплементацию node */
  typedef std::unique_ptr<node> node_ptr;
  /** \brief вектор указателей на дочерние элементы */
  typedef std::vector<node_ptr> childs_vec;

 public:
  /** \brief Добавить ноду к списку инициализированных(если можно)
   *   вытащив её параметры, в зависимости от типа ноды засунуть их
   *   в dst
   * \param src обёртка над библиотечным представление узла
   * \param dst ссылка на структуру, которую необходимо заполнить
   *   данными из src
   * \note Здесь надо вытащить имя(тип) ноды и прокинуть его
   *   в класс node_t, чтобы тонкости реализации выполнял он
   *   Ну и пока не ясно что делать с иерархичностью */
  node_sample(lib_node<NodeT> src,
              InitializerFactory* factory,
              const std::string& name)
      : BaseObject(STATUS_DEFAULT), node_(src), factory(factory), name_(name) {
    init();
  }

  /** \brief Инициализировать данные узла
   * \note about `factory->template GetNodeInitializer<NodeT>()`
   * See C++'03 Standard 14.2/4 or StackOverflow */
  void init() {
    if (factory) {
      node_data_ptr = std::unique_ptr<Initializer>(
          factory->template GetNodeInitializer<NodeT>());
      if (!node_data_ptr) {
        error_.SetError(ERROR_GENERAL_T, "Ошибка использования фабрики узлов");
        return;
      }
    } else {
      node_data_ptr = std::unique_ptr<Initializer>(new Initializer());
    }
    // инициализировать шаблон-параметр node_data_ptr и
    //   дочерние элементы(в глубину обойти)
    initData();
    child_it = childs.begin();
  }

  /** \brief Проверить наличие дочерних элементов ноды */
  NodeT* NextChild() {
    return (child_it != childs.end()) ? child_it++->get() : nullptr;
  }
  /** \brief Поиск по дочерним элементам */
  node* ChildByName(const std::string& name) const {
    node* child = nullptr;
    if (!node_data_ptr->IsLeafNode()) {
      for (const node_ptr& ch : childs) {
        if (ch->node_data_ptr->GetName() == name) {
          child = ch.get();
          break;
        }
      }
    }
    return child;
  }
  /** \brief Получить строковое представление параметра
   * \note Так-то актуально только для параметров */
  std::string GetParameter(const std::string& name) {
    return node_data_ptr->GetParameter(name);
  }
  /** \brief Получить NodeT исходник */
  const NodeT* GetSource() const { return node_.GetNodePointer(); }
  /** \brief Получить код ошибки */
  merror_t GetError() const { return error_.GetErrorCode(); }

 private:
  /** \brief Инициализировать данные ноды */
  void initData() {
    NodeT* n = node_.GetNodePointer();
    if (n) {
      /* инициализировать */
      auto error = node_data_ptr->InitData(n, name_);
      if (!error) {
        initChilds();
      } else {
        error_.SetError(error, "NodeT-> InitData finished with error");
      }
    }
  }
  /** \brief Получить список имён подузлов узла
   *   с именем 'curr_node' */
  // void ne_nujna();
  /** \brief Инициализировать дочерние элементы узла */
  void initChilds() {
    // забрать названия подузлов,
    //   хотя сейчас всё сделано так что все подузлы однотипны
    //   и собраны в один контейнер, дальновиднее подготовить
    //   вектор входных данных
    std::vector<std::string> subtrees;
    // в зависимости от типа узла название составляющих(подузлов)
    //   отличается. получим их названия
    node_data_ptr->SetSubnodesNames(&subtrees);
    // если вложенные поддеревья есть - обойдём
    for (const auto& st_name : subtrees) {
      auto ch = node_.GetChild(st_name.c_str());
      if (lib_node<NodeT>::IsInitialized(ch))
        childs.emplace_back(
            node_ptr(new node(lib_node<NodeT>(ch), factory, st_name)));
    }
    setParentData();
  }
  /** \brief Инициализировать иерархичные данные
   * \note тут такое, я пока неопределился id ноды тащить
   *   из файла конфигурации или выдавать здесь, так как без
   *   id не связываются структуры стилей из отдельного файла
   *   с базовой иерархией из основного файла */
  void setParentData() {
    for (auto& x : childs)
      x->node_data_ptr->SetParentData(*node_data_ptr);
  }

 private:
  /** \brief представление узла */
  lib_node<NodeT> node_;
  /** \brief имя узла */
  std::string name_;

 public:
  /** \brief дочерние элементы */
  childs_vec childs;
  /** \brief итератор на обход дочерних элементов */
  typename childs_vec::iterator child_it;
  // такс, все необходимые для JSONReader операции
  //   реализуем здесь
  /** \brief инициализируемая структура */
  std::unique_ptr<Initializer> node_data_ptr;
  /** \brief Фабрика */
  InitializerFactory* factory;
};

/** \brief Класс парсинга файлов
 * \note По идее здесь главным должен быть реализоывн метод
 *   позволяет вытащить весь скелет структур с++ привязанных к узлу
 *   Для случая нашего в рут нодах храняться id родительских элементов */
template <class NodeT,
          class Initializer,
          class InitializerFactory = SimpleInitializerFactory<Initializer>,
          class PathT = fs::path,
          class = typename std::enable_if<
              std::is_base_of<INodeInitializer, Initializer>::value>::type>
class ReaderSample : public BaseObject {
  ReaderSample(const ReaderSample&) = delete;
  ReaderSample operator=(const ReaderSample&) = delete;
  typedef ReaderSample<NodeT, Initializer, InitializerFactory, PathT> Reader;
  typedef node_sample<NodeT, Initializer, InitializerFactory> node;

 public:
  static ReaderSample<NodeT, Initializer, InitializerFactory, PathT>* Init(
      file_utils::FileURLSample<PathT>* source,
      InitializerFactory* factory = nullptr) {
    Reader* reader = nullptr;
    if (source) {
      if (is_exists(source->GetURL())) {
        reader = new Reader(source, factory);
      } else {
        source->SetError(ERROR_FILE_EXISTS_ST,
                         "File '" + source->GetURLStr() + "' doesn't exists");
        source->LogError();
      }
    } else {
      Logging::Append(ERROR_INIT_NULLP_ST,
                      "Get 'source'=nullptr into "
                      "Reader Init method");
    }
    return reader;
  }

  static ReaderSample<NodeT, Initializer, InitializerFactory, PathT>* Init(
      const char* data,
      InitializerFactory* factory = nullptr) {
    Reader* reader = nullptr;
    if (data) {
      reader = new Reader(data, factory);
    } else {
      Logging::Append(ERROR_INIT_NULLP_ST,
                      "Get 'data'=nullptr into "
                      "Reader Init method");
    }
    return reader;
  }

  ~ReaderSample() {
    if (memory_)
      delete[] memory_;
  }

  static std::string GetFilenameExtension() { return ".xml"; }
  /** \brief Инициализировать данные
   * \todo выносим метод из класса, сюда передаём уже данные,
   *   рут ноду, короче говоря */
  merror_t InitData() {
    if (!error_.GetErrorCode() && (memory_ != nullptr)) {
      std::string root_name = "";
      auto r = lib_node<NodeT>::InitDocumentRoot(
          &document_, memory_, len_memory_, &root_name, &error_);
      if (!error_.GetErrorCode()) {
        root_node_ = std::unique_ptr<node>(new node(r, factory_, root_name));
      }
    }
    if (error_.GetErrorCode()) {
      status_ = STATUS_HAVE_ERROR;
      error_.LogIt();
    }
    return error_.GetErrorCode();
  }

  /** \brief Получить параметр по переданному пути
   * \note Функция обобщённого обхода
   * \warning outstr придёт с пробелами, если они есть в xml,
   *   алсо путь принимается без рут ноды */
  merror_t GetValueByPath(const std::vector<std::string>& path,
                          std::string* outstr) {
    if (!root_node_)
      return ERROR_GENERAL_T;
    /* todo: добавить const квалификатор */
    node* tmp_node = root_node_.get();
    std::string param = "";
    if (!path.empty()) {
      param = path.back();
      for (auto i = path.begin(); i != path.end() - 1; ++i) {
        if (tmp_node)
          tmp_node = tmp_node->ChildByName(*i);
        else
          break;
      }
      if (!tmp_node)
        return ERROR_PARSER_CHILD_NODE_ST;
    }
    *outstr = tmp_node->GetParameter(param);
    return ERROR_SUCCESS_T;
  }

  Initializer* GetNodeByPath(const std::vector<std::string>& path) {
    if (!root_node_)
      return nullptr;
    /* todo: добавить const квалификатор */
    node* tmp_node = root_node_.get();
    std::string param = "";
    param = path.back();
    for (auto i = path.begin(); i != path.end(); ++i) {
      if (tmp_node)
        tmp_node = tmp_node->ChildByName(*i);
      else
        break;
    }
    if (!tmp_node)
      return nullptr;
    return tmp_node->node_data_ptr.get();
  }

  std::string GetFileName() { return (source_) ? source_->GetURL() : ""; }

 private:
  ReaderSample(file_utils::FileURLSample<PathT>* source,
               InitializerFactory* factory)
      : BaseObject(STATUS_DEFAULT),
        source_(source),
        memory_(nullptr),
        factory_(factory) {
    init_memory();
  }
  ReaderSample(const char* data, InitializerFactory* factory)
      : BaseObject(STATUS_DEFAULT),
        source_(nullptr),
        memory_(nullptr),
        factory_(factory) {
    init_memory(data);
  }
  /**
   * \brief Считать обрабатываемый файл в память объекта.
   *
   * \todo Копии этой функции в каждом ридере, свести к одной имплементации
   *
   * Перегрузка функции инициализации памяти для случая
   *   наличия файла в файловой системе.
   */
  void init_memory() {
    const auto content = read_file(source_->GetURL(), error_);
    init_memory(content.str().c_str());
  }
  /** \brief скопировать данные в память класса */
  void init_memory(const char* data) {
    len_memory_ = strlen(data);
    if (len_memory_ > 0) {
      memory_ = new char[len_memory_ + 1];
      memset(memory_, 0, len_memory_ + 1);
      strncpy(memory_, data, len_memory_);
    }
  }

 private:
  /** \brief адрес файла */
  file_utils::FileURLSample<PathT>* source_ = nullptr;
  /** \brief буффер памяти файла */
  char* memory_ = nullptr;
  /** \brief величина буффер памяти файла */
  size_t len_memory_ = 0;
  /** \brief основной xml объект */
  typename lib_node<NodeT>::NodeDocType document_;
  // pugi::xml_document document_;
  /** \brief корень json дерева
   * \note а в этом сетапе он наверное и не обязателен */
  std::unique_ptr<node_sample<NodeT, Initializer, InitializerFactory>>
      root_node_ = nullptr;
  /** \brief фабрика создания нод json дерева
   * \note добавить такое же в XMLReader */
  InitializerFactory* factory_ = nullptr;
};
}  // namespace asp_utils

#endif  // !UTILS__XMLREADER_H
