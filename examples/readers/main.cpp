#include "asp_utils/Common.h"
#include "asp_utils/FileURL.h"
#include "asp_utils/Readers/INode.h"
#include "asp_utils/Readers/JSONReader.h"
#include "asp_utils/Readers/Reader.h"
#include "asp_utils/Readers/XMLReader.h"
#include "inode_imp.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include <assert.h>

using namespace asp_utils;

fs::path pwd;
/*
 * todo: генерировать директорию и файлы
 */
fs::path data_dir = "example_data";
fs::path xml_file = "test_xml.xml";
fs::path json_file = "test_json.json";
file_utils::FileURLSample<fs::path>* x;
file_utils::FileURLSample<fs::path>* j;

/**
 * \brief Создать папку для данных и сгенерировать файлы
 * */
void setup_example_data() {
  ErrorWrap error;
  fs::path d = pwd / data_dir;
  fs::path xf = d / xml_file, jf = d / json_file;
  if (!fs::exists(d))
    fs::create_directory(d);
  std::ofstream fx(xf);
  fx << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<test_root name=\"test\">\n"
        "  <group name=\"first\">\n"
        "    <parameter name=\"f\"> тест юникод </parameter>\n"
        "    <parameter name=\"s\"> sdsa </parameter>\n"
        "    <parameter name=\"t\"> 116.2 </parameter>\n"
        "    <parameter name=\"ff\">  </parameter>\n"
        "  </group>\n"
        "  <group name=\"second\">\n"
        "    <parameter name=\"f\"> asd </parameter>\n"
        "    <parameter name=\"s\"> 32 </parameter>\n"
        "    <parameter name=\"t\"> 12 </parameter>\n"
        "  </group>\n"
        "</test_root>\n";
  fx.close();
  std::ofstream fy(jf);
  fy << "{\n"
        "  \"type\": \"test\",\n"
        "  \"data\": {\n"
        "    \"d1\": {\n"
        "      \"type\": \"first\",\n"
        "      \"data\": {\n"
        "        \"f\": \"тест юникод\",\n"
        "        \"s\": \"sdsa\",\n"
        "        \"t\": 116.2,\n"
        "        \"ff\": \"\"\n"
        "      }\n"
        "    },\n"
        "    \"d2\": {\n"
        "      \"type\": \"second\",\n"
        "      \"data\": {\n"
        "        \"f\": \"asd\",\n"
        "        \"s\": 32,\n"
        "        \"t\": 12\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n";
  fy.close();
}

/// unused
int xml() {
  json_test_factory tf;
  std::unique_ptr<
      XMLReaderSample<json_test_node<pugi::xml_node>, json_test_factory>>
      ss(XMLReaderSample<json_test_node<pugi::xml_node>,
                         json_test_factory>::Init(x, &tf));
  merror_t error = ss->InitData();
  return 0;
}

/// unused
int json() {
  json_test_factory tf;
  std::unique_ptr<JSONReaderSample<json_test_node<rjNValue>, json_test_factory>>
      ss(JSONReaderSample<json_test_node<rjNValue>, json_test_factory>::Init(
          j, &tf));
  merror_t error = ss->InitData();
  return 0;
}

int reader() {
  // xml
  json_test_factory tf;
  std::unique_ptr<ReaderSample<pugi::xml_node, json_test_node<pugi::xml_node>,
                               json_test_factory>>
      ss(ReaderSample<pugi::xml_node, json_test_node<pugi::xml_node>,
                      json_test_factory>::Init(x, &tf));
  merror_t e = ss->InitData();
  // json
  std::unique_ptr<
      ReaderSample<rjNValue, json_test_node<rjNValue>, json_test_factory>>
      sj(ReaderSample<rjNValue, json_test_node<rjNValue>,
                      json_test_factory>::Init(j, &tf));
  e = sj->InitData();
  return 1;
}

int main(int argc, char* argv[]) {
  Logging::InitDefault();
  pwd = fs::current_path();
  setup_example_data();
  file_utils::SetupURLSample<fs::path> s(file_utils::url_t::fs_path,
                                         (pwd / data_dir));
  file_utils::FileURLRootSample<fs::path> r(s);
  file_utils::FileURLSample sx = r.CreateFileURL(xml_file);
  file_utils::FileURLSample sj = r.CreateFileURL(json_file);
  x = &sx;
  j = &sj;
  // xml();
  // json();
  reader();
  return 0;
}
