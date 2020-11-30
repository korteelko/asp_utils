#include "FileURL.h"
#include "Readers/INode.h"
#include "Readers/JSONReader.h"
#include "Readers/Reader.h"
#include "Readers/XMLReader.h"
#include "inode_imp.h"

#include <filesystem>
#include <iostream>
#include <memory>

#include <assert.h>

namespace fs = std::filesystem;

fs::path pwd;
/*
 * todo: генерировать директорию и файлы
 */
fs::path data_dir = "data";
fs::path xml_file = "test_xml.xml";
fs::path json_file = "test_json.json";
file_utils::FileURL* x;
file_utils::FileURL* j;

int xml() {
  json_test_factory tf;
  std::unique_ptr<
      XMLReaderSample<json_test_node<pugi::xml_node>, json_test_factory>>
      ss(XMLReaderSample<json_test_node<pugi::xml_node>,
                         json_test_factory>::Init(x, &tf));
  merror_t error = ss->InitData();
  return 0;
}

int json() {
  json_test_factory tf;
  std::unique_ptr<
      JSONReaderSample<json_test_node<rj::Value>, json_test_factory>>
      ss(JSONReaderSample<json_test_node<rj::Value>, json_test_factory>::Init(
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
      ReaderSample<rj::Value, json_test_node<rj::Value>, json_test_factory>>
      sj(ReaderSample<rj::Value, json_test_node<rj::Value>,
                      json_test_factory>::Init(j, &tf));
  e = sj->InitData();
  return 1;
}

int main(int argc, char* argv[]) {
  Logging::InitDefault();
  pwd = fs::current_path();
  file_utils::SetupURL s(file_utils::url_t::fs_path, (pwd / data_dir).string());
  file_utils::FileURLRoot r(s);
  file_utils::FileURL sx = r.CreateFileURL(xml_file.string());
  file_utils::FileURL sj = r.CreateFileURL(json_file.string());
  x = &sx;
  j = &sj;
  // xml();
  // json();
  reader();
  return 0;
}
