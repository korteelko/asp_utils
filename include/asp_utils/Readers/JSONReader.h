/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__JSONREADER_H
#define UTILS__JSONREADER_H

#include "asp_utils/Base.h"
#include "asp_utils/Common.h"
#include "asp_utils/ErrorWrap.h"
#include "asp_utils/FileURL.h"
#include "asp_utils/Logging.h"
#include "asp_utils/Readers/INode.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include <fstream>
#include <functional>
#include <memory>
#include <vector>

/* json errors */
#define ERROR_JSON_T 0x0010
/// Ошибка несоответствия форматов в json файле
#define ERROR_JSON_FORMAT_ST (0x0100 | ERROR_JSON_T)
#define ERROR_JSON_FORMAT_ST_MSG "json format error "
/// Ошибка разбора json данных
#define ERROR_JSON_PARSE_ST (0x0200 | ERROR_JSON_T)
#define ERROR_JSON_PARSE_ST_MSG "json parse error "
/// Ошибка поиска подузла
#define ERROR_JSON_CHILD_INIT_ST (0x0300 | ERROR_JSON_T)
#define ERROR_JSON_CHILD_INIT_ST_MSG "json child node error "

namespace rj = rapidjson;

typedef rj::Value rjNValue;
typedef rj::Document rjNDocument;

namespace asp_utils {
/**
 * \brief Шаблон класса дерева json файла, стандартная обёртка
 *   над инициализируемой нодой
 * \note такс, для базы это структура 'control_base' или
 *   обёртка над ней: control_base_creator, а скорее всего GUIBase
 * */
template <class Initializer,
          class InitializerFactory,
          class = typename std::enable_if<
              std::is_base_of<INodeInitializer, Initializer>::value>::type>
class json_node_sample {
  typedef json_node_sample<Initializer, InitializerFactory> json_node;
  /** \brief умный указатель на имплементацию json_node */
  typedef std::unique_ptr<json_node> json_node_ptr;
  /** \brief вектор указателей на дочерние элементы */
  typedef std::vector<json_node_ptr> childs_vec;

 public:
  /** \brief Добавить ноду к списку инициализированных(если можно)
   *   вытащив её параметры, в зависимости от типа ноды засунуть их
   *   в dst
   * \param src исходный узел json файла
   * \param dst ссылка на структуру, которую необходимо заполнить
   *   данными из src
   * \note Здесь надо вытащить имя(тип) ноды и прокинуть его
   *   в класс node_t, чтобы тонкости реализации выполнял он
   *   Ну и пока не ясно что делать с иерархичностью */
  json_node_sample(rjNValue* src,
                   InitializerFactory* factory,
                   const std::string& name)
      : value_(src), factory(factory), name_(name) {
    if (factory) {
      // See C++'03 Standard 14.2/4 or StackOverflow for more
      //   information about `factory->template GetNodeInitializer<rjNValue>`
      node_data_ptr = std::unique_ptr<Initializer>(
          factory->template GetNodeInitializer<rjNValue>());
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
  json_node* NextChild() {
    return (child_it != childs.end()) ? child_it++->get() : nullptr;
  }
  /** \brief Поиск по дочерним элементам */
  json_node* ChildByName(const std::string& name) const {
    json_node* child = nullptr;
    if (!node_data_ptr->IsLeafNode()) {
      for (const json_node_ptr& ch : childs) {
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
  /** \brief Получить json исходник */
  rjNValue* GetSource() const { return value_; }
  /** \brief Получить код ошибки */
  merror_t GetError() const { return error_.GetErrorCode(); }
  /** \brief Узел - массив */
  // bool IsArray() const { return is_array_; }

 private:
  /** \brief Инициализировать данные ноды */
  void initData() {
    if (value_) {
      /* инициализировать */
      auto error = node_data_ptr->InitData(value_, name_);
      if (!error) {
        initChilds();
      } else {
        error_.SetError(error, "InitData finished with error");
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
      if (value_->HasMember(st_name.c_str())) {
        rjNValue& chs = value_->operator[](st_name.c_str());
        childs.emplace_back(
            json_node_ptr(new json_node(&chs, factory, st_name)));
      }
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
  ErrorWrap error_;
  /** \brief ссылка на представление ноды в rj */
  rjNValue* value_;
  /** \brief имя ноды */
  std::string name_;

 public:
  /** \brief ссылка на родительский элемент(unused) */
  // json_node *parent;
  /** \brief дочерние элементы */
  childs_vec childs;
  /** \brief итератор на обход дочерних элементов */
  typename childs_vec::iterator child_it;
  // такс, все необходимые для JSONReader операции
  //   реализуем здесь
  /** \brief инициализируемая структура */
  std::unique_ptr<Initializer> node_data_ptr;
  /** \brief фабрика */
  InitializerFactory* factory;
};

/** \brief Класс парсинга json файлов
 * \note По идее здесь главным должен быть реализоывн метод
 *   позволяет вытащить весь скелет структур с++ привязанных
 *   json нодам(кстати)
 *   Для случая нашего в рут нодах храняться id родительских элементов */
template <class Initializer,
          class InitializerFactory = SimpleInitializerFactory<Initializer>,
          class PathT = fs::path,
          class = typename std::enable_if<
              std::is_base_of<INodeInitializer, Initializer>::value>::type>
class JSONReaderSample : public BaseObject {
  typedef JSONReaderSample<Initializer, InitializerFactory, PathT> JSONReader;
  typedef json_node_sample<Initializer, InitializerFactory> json_node;

 public:
  JSONReaderSample(const JSONReaderSample&) = delete;
  JSONReaderSample operator=(const JSONReaderSample&) = delete;

  static JSONReaderSample<Initializer, InitializerFactory>* Init(
      file_utils::FileURLSample<PathT>* source,
      InitializerFactory* factory = nullptr) {
    JSONReader* reader = nullptr;
    if (source) {
      if (is_exists(source->GetURL())) {
        reader = new JSONReader(source, factory);
      } else {
        source->SetError(ERROR_FILE_EXISTS_ST,
                         "File '" + source->GetURLStr() + "' doesn't exists");
        source->LogError();
      }
    } else {
      Logging::Append(ERROR_INIT_NULLP_ST,
                      "Get 'source'=nullptr into "
                      "JSONReader Init method");
    }
    return reader;
  }

  static JSONReaderSample<Initializer, InitializerFactory>* Init(
      const char* data,
      InitializerFactory* factory = nullptr) {
    JSONReader* reader = nullptr;
    if (data) {
      reader = new JSONReader(data, factory);
    } else {
      Logging::Append(ERROR_INIT_NULLP_ST,
                      "Get 'data'=nullptr into "
                      "JSONReader Init method");
    }
    return reader;
  }

  ~JSONReaderSample() {
    if (memory_)
      delete[] memory_;
  }

  static std::string GetFilenameExtension() { return ".json"; }
  /** \brief Инициализировать данные */
  merror_t InitData() {
    if (!error_.GetErrorCode()) {
      // распарсить json файл
      document_.Parse(memory_);
      if (document_.HasParseError()) {
        error_.SetError(
            ERROR_JSON_FORMAT_ST,
            std::string("RapidJSON parse error: ") +
                std::string(rj::GetParseError_En(document_.GetParseError())));
      } else {
        // проверить рут
        if (document_.IsObject()) {
          auto root = document_.MemberBegin();
          root_node_ = std::unique_ptr<json_node>(
              new json_node(&root->value, factory_, root->name.GetString()));
          if (!error_.GetErrorCode())
            status_ = STATUS_OK;
        } else {
          error_.SetError(ERROR_JSON_PARSE_ST,
                          "ошибка инициализации "
                          "корневого элемента json файла " +
                              source_->GetURLStr());
        }
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
  merror_t GetValueByPath(const std::vector<std::string>& json_path,
                          std::string* outstr) {
    if (!root_node_)
      return ERROR_GENERAL_T;
    /* todo: добавить const квалификатор */
    json_node* tmp_node = root_node_.get();
    std::string param = "";
    if (!json_path.empty()) {
      param = json_path.back();
      for (auto i = json_path.begin(); i != json_path.end() - 1; ++i) {
        if (tmp_node)
          tmp_node = tmp_node->ChildByName(*i);
        else
          break;
      }
      if (!tmp_node)
        return ERROR_JSON_CHILD_INIT_ST;
    }
    *outstr = tmp_node->GetParameter(param);
    return ERROR_SUCCESS_T;
  }

  Initializer* GetNodeByPath(const std::vector<std::string>& json_path) {
    if (!root_node_)
      return nullptr;
    /* todo: добавить const квалификатор */
    json_node* tmp_node = root_node_.get();
    std::string param = "";
    param = json_path.back();
    for (auto i = json_path.begin(); i != json_path.end(); ++i) {
      if (tmp_node)
        tmp_node = tmp_node->ChildByName(*i);
      else
        break;
    }
    if (!tmp_node)
      return nullptr;
    return tmp_node->node_data_ptr.get();
  }

  std::string GetFileName() { return (source_) ? source_->GetURLStr() : ""; }

  merror_t GetErrorCode() const { return error_.GetErrorCode(); }

  // typedef std::vector<std::string> TreePath;
  /* что-то я хз */
  // rjNValue &ValueByPath(const TreePath &) {assert(0);}

 private:
  JSONReaderSample(file_utils::FileURLSample<PathT>* source,
                   InitializerFactory* factory)
      : BaseObject(STATUS_DEFAULT),
        source_(source),
        factory_(factory) {
    init_memory();
  }
  JSONReaderSample(const char* data, InitializerFactory* factory)
      : BaseObject(STATUS_DEFAULT),
        source_(nullptr),
        factory_(factory) {
    init_memory(data);
  }
  /** \brief обход дерева json объектов,
   * \note вынесено в отдельный метод потому-что можно
   *   держать лополнительное состояние, например,
   *   глубину обхода(см питоновский скрипт в asp_therm) */
  void tree_traversal(
      json_node_sample<Initializer, InitializerFactory>* jnode) {
    json_node_sample<Initializer, InitializerFactory>* child;
    // rjNValue *child_src;
    // обход дочерних элементов
    while ((child = jnode->NextChild()) != nullptr) {
      tree_traversal(child);
    }
  }
  /** \brief считать файл в память */
  void init_memory() {
    const auto content = read_file(source_->GetURL(), error_);
    init_memory(content.str().c_str());
  }
  /** \brief скопировать данные в память класса */
  void init_memory(const char* data) {
    len_memory_ = strlen(data);
    if (len_memory_ > 0) {
      memory_ = new char[len_memory_];
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
  /** \brief основной json объект */
  rjNDocument document_;
  /** \brief корень json дерева
   * \note а в этом сетапе он наверное и не обязателен */
  std::unique_ptr<json_node_sample<Initializer, InitializerFactory>> root_node_;
  /** \brief фабрика создания нод json дерева */
  InitializerFactory* factory_;
};
}  // namespace asp_utils

#endif  // !UTILS__JSONREADER_H
