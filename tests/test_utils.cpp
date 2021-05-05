#include "asp_utils/Common.h"
#include "asp_utils/ErrorWrap.h"
#include "asp_utils/FileURL.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <assert.h>


using namespace asp_utils;
using namespace asp_utils::file_utils;

TEST(Common, SplitJoin) {
  std::vector<std::string> sv0 {"qwerty", "asdfgh", "zxcvbn"};
  std::vector<std::string> sv1 {"qwerty", "", "dsgfj"};
  std::vector<std::string> sv2 {"qwerty", "", "dsgfj\"hgds"};
  std::vector<std::string> v;

  // join_container ss0
  std::stringstream ss0 = join_container(sv0, "");
  EXPECT_EQ(ss0.str(), "qwertyasdfghzxcvbn");
  ss0 = join_container(sv0, ',');
  EXPECT_EQ(ss0.str(), "qwerty,asdfgh,zxcvbn");
  split_str(ss0.str(), &v, ',');
  EXPECT_EQ(v.size(), sv0.size());
  for (size_t i = 0; i < std::min(v.size(), sv0.size()); ++i)
    EXPECT_EQ(v[i], sv0[i]);

  // join_container ss1
  std::stringstream ss1 = join_container(sv1, "");
  EXPECT_EQ(ss1.str(), "qwertydsgfj");
  ss1 = join_container(sv1, '.');
  EXPECT_EQ(ss1.str(), "qwerty..dsgfj");
  v.clear();
  split_str(ss1.str(), &v, '.');
  EXPECT_EQ(v.size(), sv1.size());
  for (size_t i = 0; i < std::min(v.size(), sv1.size()); ++i)
    EXPECT_EQ(v[i], sv1[i]);

  // join_container ss2
  std::stringstream ss2 = join_container(sv2, "");
  EXPECT_EQ(ss2.str(), "qwertydsgfj\"hgds");
  ss2 = join_container(sv2, " ,");
  EXPECT_EQ(ss2.str(), "qwerty , ,dsgfj\"hgds");
}

/**
 * \brief Тест врапера ошибок
 * */
TEST(ErrorWrap, Full) {
  ErrorWrap ew;
  /* конструктор по умолчанию */
  EXPECT_EQ(ew.GetErrorCode(), ERROR_SUCCESS_T);
  EXPECT_FALSE(ew.IsLogged());
  EXPECT_EQ(ew.GetMessage(), "");

  /* накопить изменения */
  ew.SetError(ERROR_PAIR_DEFAULT(ERROR_GENERAL_T));
  EXPECT_EQ(ew.GetErrorCode(), ERROR_GENERAL_T);
  EXPECT_FALSE(ew.GetMessage() == "");

  ew.SetErrorMessage("Тест ошибки");
  EXPECT_EQ(ew.GetMessage(), "Тест ошибки");

  ew.LogIt();
  EXPECT_TRUE(ew.IsLogged());

  /* reset */
  ew.Reset();
  EXPECT_EQ(ew.GetErrorCode(), ERROR_SUCCESS_T);
  EXPECT_FALSE(ew.IsLogged());
  EXPECT_EQ(ew.GetMessage(), "");
}

/**
 * \brief Тест FileURL
 *
 * Тест для путей представленных строками, который не реализован
 * */
TEST(FileURL, DISABLED_Full_string) {
#ifdef UNDEF
  std::string td = "test_dir";
  std::string tf = "test_file";
  fs::path test_dir(td);
  if (!fs::is_directory(test_dir)) {
    ASSERT_TRUE(fs::create_directory(test_dir));
  }
  SetupURLSample setup(url_t::fs_path, td);
  EXPECT_EQ(setup.GetURLType(), url_t::fs_path);
  EXPECT_EQ(setup.GetFullPrefix(), td);
  std::fstream file(td + "/" + tf, std::ios_base::out);
  ASSERT_TRUE(file.is_open());
  file.close();
  FileURLRoot uroot(setup);
  ASSERT_TRUE(uroot.IsInitialized());

  /* file_url */
  FileURL ufile = uroot.CreateFileURL(tf);
  EXPECT_EQ(ufile.GetError(), ERROR_SUCCESS_T);
  EXPECT_FALSE(ufile.IsInvalidPath());
  EXPECT_EQ(ufile.GetURL(), "test_dir/test_file");
  ufile.SetError(ERROR_FILE_OUT_ST, "Тест ошибки");
  EXPECT_TRUE(ufile.IsInvalidPath());

  EXPECT_TRUE(fs::remove_all(test_dir));
  #endif  // 0
}

/**
 * \brief Тест FileURL
 *
 * Тест для путей представленных fs::path
 * */
TEST(FileURL, Full_filesystem) {
  fs::path td = "test_dir";
  fs::path tf = "test_file";
  if (!fs::is_directory(td)) {
    ASSERT_TRUE(fs::create_directory(td));
  }
  SetupURLSample<fs::path> setup(url_t::fs_path, td);
  EXPECT_EQ(setup.GetURLType(), url_t::fs_path);
  EXPECT_EQ(setup.GetFullPrefix(), td);
  std::fstream file(td.string() + path_separator + tf.string(), std::ios_base::out);
  ASSERT_TRUE(file.is_open());
  file.close();
  FileURLRootSample<fs::path> uroot(setup);
  ASSERT_TRUE(uroot.IsInitialized());

  /* file_url */
  FileURLSample<fs::path> ufile = uroot.CreateFileURL(tf);
  EXPECT_EQ(ufile.GetError(), ERROR_SUCCESS_T);
  EXPECT_FALSE(ufile.IsInvalidPath());
  #ifdef OS_UNIX
  EXPECT_EQ(ufile.GetURLStr(), "test_dir/test_file");
  #endif  // OS_UNIX
  ufile.SetError(ERROR_FILE_OUT_ST, "Тест ошибки");
  EXPECT_TRUE(ufile.IsInvalidPath());

  EXPECT_TRUE(fs::remove_all(td));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
