#include "ErrorWrap.h"
#include "FileURL.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <assert.h>


using namespace file_utils;
namespace fs = std::filesystem;

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
 * */
TEST(FileURL, Full) {
  std::string td = "test_dir";
  std::string tf = "test_file";
  fs::path test_dir(td);
  if (!fs::is_directory(test_dir)) {
    ASSERT_TRUE(fs::create_directory(test_dir));
  }
  SetupURL setup(url_t::fs_path, td);
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
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
