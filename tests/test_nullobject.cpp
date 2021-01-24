#include "gtest/gtest.h"

#include "Common.h"
#include "Logging.h"

/**
 * \brief Пример имплементации интерфейса INullObject
 * */
struct nullobj : public INullObject {
  nullobj() {}
  nullobj(int i) : i(i) {}
  void Perform() override {
    status_ = STATUS_OK;
    ++i;
  }
  mstatus_t GetStatus() const { return status_; }

 public:
  mstatus_t status_ = STATUS_DEFAULT;
  int i = 1;
};

struct OptionalSharedPtrInt : public OptionalSharedPtr<int> {
 public:
  OptionalSharedPtrInt(int i) : OptionalSharedPtr<int>(i) {}
  void perform() override {
    status_ = STATUS_NOT;
    if (has_value()) {
      auto sp = get_data();
      (*sp)++;
      (*sp)++;
      status_ = STATUS_OK;
    }
  }
};

/**
 * \brief Тест наследования от INullObject
 * */
TEST(INullObject, Inherit) {
  nullobj no1;
  EXPECT_EQ(no1.GetStatus(), STATUS_DEFAULT);
  EXPECT_EQ(no1.i, 1);
  no1.Perform();
  EXPECT_EQ(no1.GetStatus(), STATUS_OK);
  EXPECT_EQ(no1.i, 2);
}

/**
 * \brief Тесты класса NullObjectSharedPtr
 * */
TEST(INullObject, NullObjectSharedPtr) {
  // тест для неудачной инициализации nullobject
  auto nos_emp = NullObjectSharedPtr<nullobj>();
  EXPECT_FALSE(nos_emp.has_value());
  EXPECT_EQ(nos_emp.status_, STATUS_NOT);
  nos_emp.perform();
  EXPECT_EQ(nos_emp.status_, STATUS_NOT);

  // тест обычный
  auto nos1 = NullObjectSharedPtr<nullobj>(std::make_shared<nullobj>(27));
  EXPECT_TRUE(nos1.has_value());
  EXPECT_EQ(nos1.status_, STATUS_DEFAULT);
  nos1.perform();
  EXPECT_EQ(nos1.status_, STATUS_OK);
  auto pnos1 = nos1.get_data();
  EXPECT_EQ(pnos1->GetStatus(), STATUS_OK);
  EXPECT_EQ(pnos1->i, 28);

  // variadic constructor
  auto nos2 = NullObjectSharedPtr<nullobj>(42);
  EXPECT_TRUE(nos2.has_value());
  EXPECT_EQ(nos2.status_, STATUS_DEFAULT);
  nos2.perform();
  EXPECT_EQ(nos2.status_, STATUS_OK);
  auto pnos2 = nos2.get_data();
  EXPECT_EQ(pnos2->GetStatus(), STATUS_OK);
  EXPECT_EQ(pnos2->i, 43);
}

TEST(OptionalSharedPtr, Inherit) {
  OptionalSharedPtrInt t(1);
  EXPECT_TRUE(t.has_value());
  EXPECT_EQ(t.status_, STATUS_DEFAULT);
  t.perform();
  EXPECT_EQ(t.status_, STATUS_OK);
  EXPECT_EQ(*t.get_data(), 3);
}
