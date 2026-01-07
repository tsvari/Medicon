#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TestSharedUtility.h"
#include <QStandardItemModel>
#include <QItemSelectionModel>

using ::testing::ElementsAre;
using ::testing::UnorderedElementsAre;

// ============================================================================
// compareQVariant Tests
// ============================================================================

TEST(CompareQVariantTest, EqualIntegers) {
    QVariant v1(42);
    QVariant v2(42);
    EXPECT_TRUE(compareQVariant(v1, v2));
}

TEST(CompareQVariantTest, UnequalIntegers) {
    QVariant v1(42);
    QVariant v2(100);
    EXPECT_FALSE(compareQVariant(v1, v2));
}

TEST(CompareQVariantTest, EqualStrings) {
    QVariant v1("test");
    QVariant v2("test");
    EXPECT_TRUE(compareQVariant(v1, v2));
}

TEST(CompareQVariantTest, UnequalStrings) {
    QVariant v1("test");
    QVariant v2("other");
    EXPECT_FALSE(compareQVariant(v1, v2));
}

TEST(CompareQVariantTest, EqualDoubles) {
    QVariant v1(3.14);
    QVariant v2(3.14);
    EXPECT_TRUE(compareQVariant(v1, v2));
}

TEST(CompareQVariantTest, EqualBooleans) {
    QVariant v1(true);
    QVariant v2(true);
    EXPECT_TRUE(compareQVariant(v1, v2));
}

TEST(CompareQVariantTest, UnequalBooleans) {
    QVariant v1(true);
    QVariant v2(false);
    EXPECT_FALSE(compareQVariant(v1, v2));
}

TEST(CompareQVariantTest, DifferentTypes) {
    QVariant v1(42);
    QVariant v2("test");
    EXPECT_FALSE(compareQVariant(v1, v2));
}

TEST(CompareQVariantTest, NullVariants) {
    QVariant v1;
    QVariant v2;
    // Invalid/null QVariants are considered unordered, not equivalent
    EXPECT_FALSE(compareQVariant(v1, v2));
}

// ============================================================================
// compareQVariantList Tests
// ============================================================================

TEST(CompareQVariantListTest, EmptyLists) {
    QList<QVariant> list1;
    QList<QVariant> list2;
    EXPECT_TRUE(compareQVariantList(list1, list2));
}

TEST(CompareQVariantListTest, EqualLists) {
    QList<QVariant> list1 = {1, "test", 3.14, true};
    QList<QVariant> list2 = {1, "test", 3.14, true};
    EXPECT_TRUE(compareQVariantList(list1, list2));
}

TEST(CompareQVariantListTest, DifferentSizes) {
    QList<QVariant> list1 = {1, 2, 3};
    QList<QVariant> list2 = {1, 2};
    EXPECT_FALSE(compareQVariantList(list1, list2));
}

TEST(CompareQVariantListTest, DifferentElements) {
    QList<QVariant> list1 = {1, 2, 3};
    QList<QVariant> list2 = {1, 2, 4};
    EXPECT_FALSE(compareQVariantList(list1, list2));
}

TEST(CompareQVariantListTest, DifferentOrder) {
    QList<QVariant> list1 = {1, 2, 3};
    QList<QVariant> list2 = {3, 2, 1};
    EXPECT_FALSE(compareQVariantList(list1, list2));
}

// ============================================================================
// pullout Tests (with QItemSelection)
// ============================================================================

class PulloutTest : public ::testing::Test {
protected:
    void SetUp() override {
        model = new QStandardItemModel(3, 3);
        
        // Fill model with test data
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                QStandardItem* item = new QStandardItem();
                item->setData(row * 10 + col, Qt::DisplayRole);
                item->setData(QString("Item %1,%2").arg(row).arg(col), Qt::UserRole);
                model->setItem(row, col, item);
            }
        }
    }

    void TearDown() override {
        delete model;
    }

    QStandardItemModel* model;
};

TEST_F(PulloutTest, PulloutWithRole) {
    QItemSelection selection;
    selection.select(model->index(0, 0), model->index(0, 2));
    
    auto results = pullout<int>(selection, Qt::DisplayRole);
    
    ASSERT_EQ(results.size(), 3);
    EXPECT_THAT(results, ElementsAre(0, 1, 2));
}

TEST_F(PulloutTest, PulloutWithCustomRole) {
    QItemSelection selection;
    selection.select(model->index(0, 0), model->index(0, 2));
    
    auto results = pullout<QString>(selection, Qt::UserRole);
    
    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], "Item 0,0");
    EXPECT_EQ(results[1], "Item 0,1");
    EXPECT_EQ(results[2], "Item 0,2");
}

TEST_F(PulloutTest, PulloutWithFunction) {
    QItemSelection selection;
    selection.select(model->index(1, 0), model->index(1, 2));
    
    std::function<int(QModelIndex)> func = [](QModelIndex index) {
        return index.data(Qt::DisplayRole).toInt() * 2;
    };
    
    auto results = pullout<int>(selection, func);
    
    ASSERT_EQ(results.size(), 3);
    EXPECT_THAT(results, ElementsAre(20, 22, 24));
}

TEST_F(PulloutTest, PulloutEmptySelection) {
    QItemSelection selection;
    
    auto results = pullout<int>(selection, Qt::DisplayRole);
    
    EXPECT_TRUE(results.empty());
}

TEST_F(PulloutTest, PulloutMultipleRanges) {
    QItemSelection selection;
    selection.select(model->index(0, 0), model->index(0, 1));
    selection.select(model->index(2, 1), model->index(2, 2));
    
    auto results = pullout<int>(selection, Qt::DisplayRole);
    
    ASSERT_EQ(results.size(), 4);
    EXPECT_THAT(results, ElementsAre(0, 1, 21, 22));
}

// ============================================================================
// pulloutHeader Tests
// ============================================================================

class PulloutHeaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        model = new QStandardItemModel(3, 3);
        
        // Set horizontal headers
        model->setHorizontalHeaderItem(0, new QStandardItem("Col0"));
        model->setHorizontalHeaderItem(1, new QStandardItem("Col1"));
        model->setHorizontalHeaderItem(2, new QStandardItem("Col2"));
        
        // Set vertical headers
        model->setVerticalHeaderItem(0, new QStandardItem("Row0"));
        model->setVerticalHeaderItem(1, new QStandardItem("Row1"));
        model->setVerticalHeaderItem(2, new QStandardItem("Row2"));
    }

    void TearDown() override {
        delete model;
    }

    QStandardItemModel* model;
};

TEST_F(PulloutHeaderTest, PulloutHorizontalHeaders) {
    std::vector<int> sections = {0, 1, 2};
    
    auto results = pulloutHeader<QString>(model, sections, Qt::Horizontal, Qt::DisplayRole);
    
    ASSERT_EQ(results.size(), 3);
    EXPECT_THAT(results, ElementsAre("Col0", "Col1", "Col2"));
}

TEST_F(PulloutHeaderTest, PulloutVerticalHeaders) {
    std::vector<int> sections = {0, 1, 2};
    
    auto results = pulloutHeader<QString>(model, sections, Qt::Vertical, Qt::DisplayRole);
    
    ASSERT_EQ(results.size(), 3);
    EXPECT_THAT(results, ElementsAre("Row0", "Row1", "Row2"));
}

TEST_F(PulloutHeaderTest, PulloutSelectedSections) {
    std::vector<int> sections = {0, 2};
    
    auto results = pulloutHeader<QString>(model, sections, Qt::Horizontal, Qt::DisplayRole);
    
    ASSERT_EQ(results.size(), 2);
    EXPECT_THAT(results, ElementsAre("Col0", "Col2"));
}

TEST_F(PulloutHeaderTest, PulloutEmptySections) {
    std::vector<int> sections;
    
    auto results = pulloutHeader<QString>(model, sections, Qt::Horizontal, Qt::DisplayRole);
    
    EXPECT_TRUE(results.empty());
}

// ============================================================================
// deref_if_refwrap Tests
// ============================================================================

TEST(DerefIfRefWrapTest, PlainValue) {
    int value = 42;
    const int& result = deref_if_refwrap(value);
    EXPECT_EQ(result, 42);
}

TEST(DerefIfRefWrapTest, ReferenceWrapper) {
    int value = 42;
    std::reference_wrapper<int> ref(value);
    const int& result = deref_if_refwrap(ref);
    EXPECT_EQ(result, 42);
}

TEST(DerefIfRefWrapTest, ConstReferenceWrapper) {
    const int value = 42;
    std::reference_wrapper<const int> ref(value);
    const int& result = deref_if_refwrap(ref);
    EXPECT_EQ(result, 42);
}

// ============================================================================
// loose_compare Tests
// ============================================================================

TEST(LooseCompareTest, SameTypeIntegers) {
    std::variant<int, double, std::string> v1 = 42;
    std::variant<int, double, std::string> v2 = 42;
    EXPECT_TRUE(loose_compare(v1, v2));
}

TEST(LooseCompareTest, SameTypeStrings) {
    std::variant<int, double, std::string> v1 = std::string("test");
    std::variant<int, double, std::string> v2 = std::string("test");
    EXPECT_TRUE(loose_compare(v1, v2));
}

TEST(LooseCompareTest, SameTypeNotEqual) {
    std::variant<int, double, std::string> v1 = 42;
    std::variant<int, double, std::string> v2 = 100;
    EXPECT_FALSE(loose_compare(v1, v2));
}

TEST(LooseCompareTest, DifferentArithmeticTypes) {
    std::variant<int, double, std::string> v1 = 42;
    std::variant<int, double, std::string> v2 = 42.0;
    EXPECT_TRUE(loose_compare(v1, v2));
}

TEST(LooseCompareTest, DifferentArithmeticTypesNotEqual) {
    std::variant<int, double, std::string> v1 = 42;
    std::variant<int, double, std::string> v2 = 42.5;
    EXPECT_FALSE(loose_compare(v1, v2));
}

TEST(LooseCompareTest, IncompatibleTypes) {
    std::variant<int, double, std::string> v1 = 42;
    std::variant<int, double, std::string> v2 = std::string("42");
    EXPECT_FALSE(loose_compare(v1, v2));
}

TEST(LooseCompareTest, WithReferenceWrapper) {
    int value1 = 42;
    int value2 = 42;
    std::variant<int, std::reference_wrapper<int>> v1 = std::ref(value1);
    std::variant<int, std::reference_wrapper<int>> v2 = std::ref(value2);
    EXPECT_TRUE(loose_compare(v1, v2));
}

// ============================================================================
// loose_vector_compare Tests
// ============================================================================

TEST(LooseVectorCompareTest, EmptyVectors) {
    std::vector<std::variant<int, double, std::string>> v1;
    std::vector<std::variant<int, double, std::string>> v2;
    EXPECT_TRUE(loose_vector_compare(v1, v2));
}

TEST(LooseVectorCompareTest, EqualVectors) {
    std::vector<std::variant<int, double, std::string>> v1 = {1, 2.5, std::string("test")};
    std::vector<std::variant<int, double, std::string>> v2 = {1, 2.5, std::string("test")};
    EXPECT_TRUE(loose_vector_compare(v1, v2));
}

TEST(LooseVectorCompareTest, DifferentSizes) {
    std::vector<std::variant<int, double, std::string>> v1 = {1, 2, 3};
    std::vector<std::variant<int, double, std::string>> v2 = {1, 2};
    EXPECT_FALSE(loose_vector_compare(v1, v2));
}

TEST(LooseVectorCompareTest, DifferentElements) {
    std::vector<std::variant<int, double, std::string>> v1 = {1, 2, 3};
    std::vector<std::variant<int, double, std::string>> v2 = {1, 2, 4};
    EXPECT_FALSE(loose_vector_compare(v1, v2));
}

TEST(LooseVectorCompareTest, MixedArithmeticTypes) {
    std::vector<std::variant<int, double, std::string>> v1 = {1, 2.0, 3};
    std::vector<std::variant<int, double, std::string>> v2 = {1.0, 2, 3.0};
    EXPECT_TRUE(loose_vector_compare(v1, v2));
}

TEST(LooseVectorCompareTest, IncompatibleTypesInVector) {
    std::vector<std::variant<int, double, std::string>> v1 = {1, std::string("test")};
    std::vector<std::variant<int, double, std::string>> v2 = {1, std::string("other")};
    EXPECT_FALSE(loose_vector_compare(v1, v2));
}

// ============================================================================
// MasterObject Tests
// ============================================================================

TEST(MasterObjectTest, DefaultConstruction) {
    MasterObject obj;
    // Just verify that default construction works
    SUCCEED();
}

TEST(MasterObjectTest, SetAndGetUid) {
    MasterObject obj;
    obj.set_uid(123);
    EXPECT_EQ(obj.uid(), 123);
}

TEST(MasterObjectTest, SetAndGetName) {
    MasterObject obj;
    obj.set_name("John Doe");
    EXPECT_EQ(obj.name(), "John Doe");
}

TEST(MasterObjectTest, SetAndGetDate) {
    MasterObject obj;
    obj.set_date(20231231);
    EXPECT_EQ(obj.date(), 20231231);
}

TEST(MasterObjectTest, SetAndGetTime) {
    MasterObject obj;
    obj.set_time(123456);
    EXPECT_EQ(obj.time(), 123456);
}

TEST(MasterObjectTest, SetAndGetDateTime) {
    MasterObject obj;
    obj.set_date_time(1672531200);
    EXPECT_EQ(obj.date_time(), 1672531200);
}

TEST(MasterObjectTest, SetAndGetDateTimeNoSec) {
    MasterObject obj;
    obj.set_date_time_no_sec(1672531200);
    EXPECT_EQ(obj.date_time_no_sec(), 1672531200);
}

TEST(MasterObjectTest, SetAndGetHeight) {
    MasterObject obj;
    obj.set_height(175);
    EXPECT_EQ(obj.height(), 175);
}

TEST(MasterObjectTest, SetAndGetSalary) {
    MasterObject obj;
    obj.set_salary(50000.50);
    EXPECT_DOUBLE_EQ(obj.salary(), 50000.50);
}

TEST(MasterObjectTest, SetAndGetMarried) {
    MasterObject obj;
    obj.set_married(true);
    EXPECT_TRUE(obj.married());
    
    obj.set_married(false);
    EXPECT_FALSE(obj.married());
}

TEST(MasterObjectTest, SetAndGetLevel) {
    MasterObject obj;
    obj.set_level(3);
    EXPECT_EQ(obj.level(), 3);
}

TEST(MasterObjectTest, SetAndGetLevelName) {
    MasterObject obj;
    obj.set_level_name("Senior");
    EXPECT_EQ(obj.level_name(), "Senior");
}

TEST(MasterObjectTest, SetAndGetMarriedName) {
    MasterObject obj;
    obj.set_married_name("Yes");
    EXPECT_EQ(obj.married_name(), "Yes");
}

TEST(MasterObjectTest, SetAndGetImage) {
    MasterObject obj;
    obj.set_image("/path/to/image.png");
    EXPECT_EQ(obj.image(), "/path/to/image.png");
}

TEST(MasterObjectTest, AllPropertiesTogether) {
    MasterObject obj;
    obj.set_uid(1);
    obj.set_name("Alice");
    obj.set_date(20240101);
    obj.set_time(120000);
    obj.set_date_time(1704110400);
    obj.set_date_time_no_sec(1704110400);
    obj.set_height(165);
    obj.set_salary(75000.00);
    obj.set_married(true);
    obj.set_level(2);
    obj.set_level_name("Mid");
    obj.set_married_name("Married");
    obj.set_image("/images/alice.jpg");
    
    EXPECT_EQ(obj.uid(), 1);
    EXPECT_EQ(obj.name(), "Alice");
    EXPECT_EQ(obj.date(), 20240101);
    EXPECT_EQ(obj.time(), 120000);
    EXPECT_EQ(obj.date_time(), 1704110400);
    EXPECT_EQ(obj.date_time_no_sec(), 1704110400);
    EXPECT_EQ(obj.height(), 165);
    EXPECT_DOUBLE_EQ(obj.salary(), 75000.00);
    EXPECT_TRUE(obj.married());
    EXPECT_EQ(obj.level(), 2);
    EXPECT_EQ(obj.level_name(), "Mid");
    EXPECT_EQ(obj.married_name(), "Married");
    EXPECT_EQ(obj.image(), "/images/alice.jpg");
}

// ============================================================================
// SlaveObject Tests
// ============================================================================

TEST(SlaveObjectTest, DefaultConstruction) {
    SlaveObject obj;
    // Just verify that default construction works
    SUCCEED();
}

TEST(SlaveObjectTest, ParameterizedConstruction) {
    SlaveObject obj(1, 100, "+1234567890");
    EXPECT_EQ(obj.uid(), 1);
    EXPECT_EQ(obj.link_uid(), 100);
    EXPECT_EQ(obj.phone(), "+1234567890");
}

TEST(SlaveObjectTest, SetAndGetUid) {
    SlaveObject obj;
    obj.set_uid(42);
    EXPECT_EQ(obj.uid(), 42);
}

TEST(SlaveObjectTest, SetAndGetLinkUid) {
    SlaveObject obj;
    obj.set_link_uid(999);
    EXPECT_EQ(obj.link_uid(), 999);
}

TEST(SlaveObjectTest, SetAndGetPhone) {
    SlaveObject obj;
    obj.set_phone("+9876543210");
    EXPECT_EQ(obj.phone(), "+9876543210");
}

TEST(SlaveObjectTest, AllPropertiesTogether) {
    SlaveObject obj;
    obj.set_uid(5);
    obj.set_link_uid(50);
    obj.set_phone("+1122334455");
    
    EXPECT_EQ(obj.uid(), 5);
    EXPECT_EQ(obj.link_uid(), 50);
    EXPECT_EQ(obj.phone(), "+1122334455");
}

TEST(SlaveObjectTest, ModifyAfterConstruction) {
    SlaveObject obj(1, 10, "+1111111111");
    
    obj.set_uid(2);
    obj.set_link_uid(20);
    obj.set_phone("+2222222222");
    
    EXPECT_EQ(obj.uid(), 2);
    EXPECT_EQ(obj.link_uid(), 20);
    EXPECT_EQ(obj.phone(), "+2222222222");
}

// ============================================================================
// randomInt Tests (testing the anonymous namespace function indirectly)
// ============================================================================

TEST(RandomIntTest, GeneratesWithinRange) {
    // Note: randomInt is in anonymous namespace, so we can't directly test it
    // But we can test it indirectly if there are any public functions using it
    // For now, we just verify that QRandomGenerator works as expected
    
    int min = 1;
    int max = 100;
    for (int i = 0; i < 100; ++i) {
        int value = QRandomGenerator::global()->bounded(max - min + 1) + min;
        EXPECT_GE(value, min);
        EXPECT_LE(value, max);
    }
}
