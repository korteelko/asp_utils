/**
 * utils
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__XMLREADER_H
#define UTILS__XMLREADER_H

#include "asp_utils/Base.h"
#include "asp_utils/Common.h"
#include "asp_utils/ErrorWrap.h"
#include "asp_utils/FileURL.h"
#include "asp_utils/Logging.h"
#include "asp_utils/Readers/INode.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <string.h>

namespace asp_utils {
/**
 * \brief шаблон класса дерева файла, стандартная обёртка
 *   над инициализируемой нодой
 * */
template <class Initializer,
          class InitializerFactory,
          class = typename std::enable_if<
              std::is_base_of<INodeInitializer, Initializer>::value>::type>
class xml_node_sample {
  typedef xml_node_sample<Initializer, InitializerFactory> xml_node;
  /** \brief умный указатель на имплементацию xml_node */
  typedef std::unique_ptr<xml_node> xml_node_ptr;
  /** \brief вектор указателей на дочерние элементы */
  typedef std::vector<xml_node_ptr> childs_vec;

 public:
  /** \brief Добавить ноду к списку инициализированных(если можно)
   *   вытащив её параметры, в зависимости от типа ноды засунуть их
   *   в dst
   * \param src исходный узел xml файла
   * \param dst ссылка на структуру, которую необходимо заполнить
   *   данными из src
   * \note Здесь надо вытащить имя(тип) ноды и прокинуть его
   *   в класс node_t, чтобы тонкости реализации выполнял он
   *   Ну и пока не ясно что делать с иерархичностью */
  xml_node_sample(pugi::xml_node* src, InitializerFactory* factory)
      : node_(*src), factory(factory) {
    init();
  }

  xml_node_sample(pugi::xml_node&& src, InitializerFactory* factory)
      : BaseObject(STATUS_DEFAULT), node_(src), factory(factory) {
    init();
  }

  void init() {
    if (factory) {
      node_data_ptr = std::unique_ptr<Initializer>(
          factory->template GetNodeInitializer<pugi::xml_node>());
      if (!node_data_ptr) {
        error_.SetError(ERROR_GENERAL_T, "Ошибка использования фабрики узлов");
        return;
      }
    } else {
      node_data_ptr = std::unique_ptr<Initializer>(new Initializer());
    }
    initData();
    child_it = childs.begin();
  }

  /** \brief Проверить наличие дочерних элементов ноды */
  xml_node* NextChild() {
    return (child_it != childs.end()) ? child_it++->get() : nullptr;
  }
  /** \brief Поиск по дочерним элементам */
  xml_node* ChildByName(const std::string& name) const {
    xml_node* child = nullptr;
    if (!node_data_ptr->IsLeafNode()) {
      for (const xml_node_ptr& ch : childs) {
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
  /** \brief Получить xml исходник */
  const pugi::xml_node* GetSource() const { return &node_; }
  /** \brief Получить код ошибки */
  merror_t GetError() const { return error_.GetErrorCode(); }

 private:
  /** \brief Инициализировать данные ноды */
  void initData() {
    if (!node_.empty()) {
      std::string name(node_.name());
      /* инициализировать */
      auto error = node_data_ptr->InitData(&node_, name);
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
      pugi::xml_node&& ch = node_.child(st_name.c_str());
      // для xml хдесь попроще
      if (!ch.empty())
        childs.emplace_back(xml_node_ptr(new xml_node(&ch, factory)));
    }
    setParentData();
  }
  /** \brief Инициализировать иерархичные данные
   * \note тут такое, я пока неопределился id ноды тащить
   *   из файла конфигурации или выдавать здесь, так как без
   *   id не связываются структуры стилей из отдельного файла
   *   с базовой иерархией из основного файла */
  void setParentData() {
    for (auto& child : childs)
      child->node_data_ptr->SetParentData(*node_data_ptr);
  }

 private:
  ErrorWrap error_;
  /** \brief представление узла */
  pugi::xml_node node_;
  /** \brief имя узла */
  // std::string name_;

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

/** \brief Класс парсинга xml файлов
 * \note По идее здесь главным должен быть реализоывн метод
 *   позволяет вытащить весь скелет структур с++ привязанных к узлу
 *   Для случая нашего в рут нодах храняться id родительских элементов */
template <class Initializer,
          class InitializerFactory = SimpleInitializerFactory<Initializer>,
          class PathT = fs::path,
          class = typename std::enable_if<
              std::is_base_of<INodeInitializer, Initializer>::value>::type>
class XMLReaderSample : public BaseObject {
  XMLReaderSample(const XMLReaderSample&) = delete;
  XMLReaderSample operator=(const XMLReaderSample&) = delete;
  typedef XMLReaderSample<Initializer, InitializerFactory, PathT> XMLReader;
  typedef xml_node_sample<Initializer, InitializerFactory> xml_node;

 public:
  static XMLReaderSample<Initializer, InitializerFactory>* Init(
      file_utils::FileURLSample<PathT>* source,
      InitializerFactory* factory = nullptr) {
    XMLReader* reader = nullptr;
    if (source) {
      if (is_exists(source->GetURL())) {
        reader = new XMLReader(source, factory);
      } else {
        source->SetError(ERROR_FILE_EXISTS_ST,
                         "File '" + source->GetURLStr() + "' doesn't exists");
        source->LogError();
      }
    } else {
      Logging::Append(ERROR_INIT_NULLP_ST,
                      "Get 'source'=nullptr into "
                      "XMLReader Init method");
    }
    return reader;
  }

  static XMLReaderSample<Initializer, InitializerFactory>* Init(
      const char* data,
      InitializerFactory* factory = nullptr) {
    XMLReader* reader = nullptr;
    if (data) {
      reader = new XMLReader(data, factory);
    } else {
      Logging::Append(ERROR_INIT_NULLP_ST,
                      "Get 'data'=nullptr into "
                      "XMLReader Init method");
    }
    return reader;
  }

  ~XMLReaderSample() {
    if (memory_)
      delete[] memory_;
  }

  static std::string GetFilenameExtension() { return ".xml"; }
  /** \brief Инициализировать данные */
  merror_t InitData() {
    if (!error_.GetErrorCode() && (memory_ != nullptr)) {
      pugi::xml_parse_result res =
          document_.load_buffer_inplace(memory_, len_memory_);
      if (!res) {
        // ошибка разбора документа
        error_.SetError(
            ERROR_PARSER_FORMAT_ST,
            std::string("pugixml parse error: ") + res.description());
      } else {
        pugi::xml_node r = *document_.begin();
        root_node_ = std::unique_ptr<xml_node>(new xml_node(&r, factory_));
        if (!error_.GetErrorCode())
          status_ = STATUS_OK;
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
  merror_t GetValueByPath(const std::vector<std::string>& xml_path,
                          std::string* outstr) {
    if (!root_node_)
      return ERROR_GENERAL_T;
    /* todo: добавить const квалификатор */
    xml_node* tmp_node = root_node_.get();
    std::string param = "";
    if (!xml_path.empty()) {
      param = xml_path.back();
      for (auto i = xml_path.begin(); i != xml_path.end() - 1; ++i) {
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

  Initializer* GetNodeByPath(const std::vector<std::string>& xml_path) {
    if (!root_node_)
      return nullptr;
    /* todo: добавить const квалификатор */
    xml_node* tmp_node = root_node_.get();
    std::string param = "";
    param = xml_path.back();
    for (auto i = xml_path.begin(); i != xml_path.end(); ++i) {
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

  void LogError() { error_.LogIt(); }

 private:
  XMLReaderSample(file_utils::FileURLSample<PathT>* source,
                  InitializerFactory* factory)
      : BaseObject(STATUS_DEFAULT),
        source_(source),
        factory_(factory) {
    init_memory();
  }
  XMLReaderSample(const char* data, InitializerFactory* factory)
      : BaseObject(STATUS_DEFAULT),
        source_(nullptr),
        factory_(factory) {
    init_memory(data);
  }
  /** \brief обход дерева xml объектов,
   * \note вынесено в отдельный метод потому-что можно
   *   держать лополнительное состояние, например,
   *   глубину обхода(см питоновский скрипт в asp_therm) */
  void tree_traversal(xml_node_sample<Initializer, InitializerFactory>* xnode) {
    xml_node_sample<Initializer, InitializerFactory>* child;
    while ((child = xnode->NextChild()) != nullptr) {
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
  pugi::xml_document document_;
  /** \brief корень json дерева
   * \note а в этом сетапе он наверное и не обязателен */
  std::unique_ptr<xml_node_sample<Initializer, InitializerFactory>> root_node_ =
      nullptr;
  /** \brief фабрика создания нод json дерева
   * \note добавить такое же в XMLReader */
  InitializerFactory* factory_ = nullptr;
};
}  // namespace asp_utils

#endif  // !UTILS__XMLREADER_H
