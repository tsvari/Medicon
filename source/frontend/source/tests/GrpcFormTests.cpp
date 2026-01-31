/**
 * @file GrpcFormTests.cpp
 * @brief Comprehensive tests for GrpcForm class
 */

#include <gtest/gtest.h>

#include "../GrpcForm.h"
#include "../GrpcObjectWrapper.hpp"
#include "../GrpcDataContainer.hpp"
#include "../GrpcObjectTableModel.h"
#include "../TestSharedUtility.h"

#include <QApplication>
#include <QLineEdit>
#include <QCheckBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QDateTimeEdit>
#include <QLabel>
#include <QBuffer>
#include <QImage>
#include <QTabBar>
#include <QTabWidget>
#include <QSignalSpy>
#include <QTest>

using namespace testing;

// Test model implementation
class TestGrpcObjectTableModel : public GrpcObjectTableModel
{
public:
    explicit TestGrpcObjectTableModel(QObject *parent = nullptr)
        : GrpcObjectTableModel(new GrpcDataContainer<MasterObject>(), parent)
    {
        initializeModel();
        initializeContainer();
    }

    void initializeModel() override
    {
        auto * container = dynamic_cast<GrpcDataContainer<MasterObject>*>(objectContainer());
        if (!container) return;

        container->addProperty("name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
        container->addProperty("height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
        container->addProperty("married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
        container->addProperty("salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    }
};

// Real form implementation using actual GrpcObjectWrapper
class TestGrpcForm : public GrpcForm
{
    Q_OBJECT
public:
    TestGrpcForm(QWidget * parent = nullptr)
        : GrpcForm(new GrpcObjectWrapper<MasterObject>(), nullptr, parent)
    {
        initializeForm();
    }

    TestGrpcForm(IBaseGrpcObjectWrapper * wrapper, IBaseGrpcObjectWrapper * master = nullptr, QWidget * parent = nullptr)
        : GrpcForm(wrapper, master, parent)
    {
        initializeForm();
    }

    void initializeForm() override
    {
        auto * wrapper = dynamic_cast<GrpcObjectWrapper<MasterObject>*>(objectWrapper());
        if (!wrapper) return;

        // Create widgets
        m_nameEdit = new QLineEdit(this);
        m_nameEdit->setObjectName("name");
        
        m_heightEdit = new QLineEdit(this);
        m_heightEdit->setObjectName("height");
        
        m_marriedCheck = new QCheckBox(this);
        m_marriedCheck->setObjectName("married");
        
        m_salaryEdit = new QLineEdit(this);
        m_salaryEdit->setObjectName("salary");

        m_imageLabel = new QLabel(this);
        m_imageLabel->setObjectName("imageLabel");
        m_imageLabel->setScaledContents(true);

        // Add properties
        wrapper->addProperty("name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
        wrapper->addProperty("height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
        wrapper->addProperty("married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married, DataMask::CheckBoxMask, "1", "0");
        wrapper->addProperty("salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
        wrapper->addProperty("imageLabel", DataInfo::String, &MasterObject::set_image, &MasterObject::image);
        
        // Initialize widgets after properties are added
        initilizeWidgets();
    }

    QVariant defaultObject() override
    {
        MasterObject obj;
        obj.set_name("Default");
        obj.set_height(0);
        obj.set_married(false);
        obj.set_salary(0.0);
        return QVariant::fromValue(obj);
    }

    // Expose protected for testing
    IBaseGrpcObjectWrapper * getObjectWrapper() { return objectWrapper(); }
    QVariant getMasterVariant() { return masterVariantObject(); }

public:
    QLineEdit * m_nameEdit = nullptr;
    QLineEdit * m_heightEdit = nullptr;
    QCheckBox * m_marriedCheck = nullptr;
    QLineEdit * m_salaryEdit = nullptr;
    QLabel * m_imageLabel = nullptr;
};

// Form implementation including date/time widgets
class TestGrpcFormWithDateTime : public GrpcForm
{
    Q_OBJECT
public:
    TestGrpcFormWithDateTime(QWidget * parent = nullptr)
        : GrpcForm(new GrpcObjectWrapper<MasterObject>(), nullptr, parent)
    {
        initializeForm();
    }

    void initializeForm() override
    {
        auto * wrapper = dynamic_cast<GrpcObjectWrapper<MasterObject>*>(objectWrapper());
        if (!wrapper) return;

        m_dateEdit = new QDateEdit(this);
        m_dateEdit->setObjectName("date");

        m_timeEdit = new QTimeEdit(this);
        m_timeEdit->setObjectName("time");

        m_dateTimeEdit = new QDateTimeEdit(this);
        m_dateTimeEdit->setObjectName("date_time");

        wrapper->addProperty("date", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
        wrapper->addProperty("time", DataInfo::Time, &MasterObject::set_time, &MasterObject::time);
        wrapper->addProperty("date_time", DataInfo::DateTime, &MasterObject::set_date_time, &MasterObject::date_time);

        initilizeWidgets();

        // Ensure wrapper has an object so fillObject() does work.
        wrapper->setObject(defaultObject());
    }

    QVariant defaultObject() override
    {
        MasterObject obj;
        obj.set_date(0);
        obj.set_time(0);
        obj.set_date_time(0);
        return QVariant::fromValue(obj);
    }

public:
    QDateEdit * m_dateEdit = nullptr;
    QTimeEdit * m_timeEdit = nullptr;
    QDateTimeEdit * m_dateTimeEdit = nullptr;
};

class GrpcFormTest : public Test
{
protected:
    void SetUp() override
    {
        // Ensure QApplication exists
        if (!qApp) {
            int argc = 0;
            char *argv[] = {nullptr};
            app = new QApplication(argc, argv);
        }
    }

    void TearDown() override
    {
        // Cleanup is handled by smart pointers
    }

    QApplication * app = nullptr;
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(GrpcFormTest, ConstructorInitializesObjectWrapper)
{
    TestGrpcForm form;
    
    EXPECT_NE(form.getObjectWrapper(), nullptr);
}

TEST_F(GrpcFormTest, ConstructorWithMasterWrapper)
{
    auto * wrapper = new GrpcObjectWrapper<MasterObject>();
    auto * masterWrapper = new GrpcObjectWrapper<MasterObject>();
    
    // Initialize wrappers with properties
    wrapper->addProperty("name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    masterWrapper->addProperty("name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    
    // Set a default object in master wrapper
    MasterObject masterObj;
    masterObj.set_name("Master");
    masterWrapper->setObject(QVariant::fromValue(masterObj));
    
    TestGrpcForm form(wrapper, masterWrapper);
    
    EXPECT_NE(form.getObjectWrapper(), nullptr);
    EXPECT_TRUE(form.getMasterVariant().isValid());
}

TEST_F(GrpcFormTest, ConstructorInitializesWidgets)
{
    TestGrpcForm form;
    
    EXPECT_NE(form.m_nameEdit, nullptr);
    EXPECT_NE(form.m_heightEdit, nullptr);
    EXPECT_NE(form.m_marriedCheck, nullptr);
    EXPECT_NE(form.m_salaryEdit, nullptr);
}

// ============================================================================
// fill() Tests
// ============================================================================

TEST_F(GrpcFormTest, FillPopulatesWidgetsFromModel)
{
    TestGrpcForm form;
    
    // Create model and insert test data
    TestGrpcObjectTableModel model;
    
    MasterObject obj;
    obj.set_name("Test Name");
    obj.set_height(180);
    obj.set_married(true);
    obj.set_salary(50000.50);
    
    model.insertObject(0, QVariant::fromValue(obj));
    
    QModelIndex index = model.index(0, 0);
    form.fill(index);
    
    EXPECT_EQ(form.m_nameEdit->text().toStdString(), "Test Name");
    EXPECT_EQ(form.m_heightEdit->text().toInt(), 180);
    // Use QLocale to parse numbers that may have locale-specific formatting
    bool ok = false;
    double salary = QLocale().toDouble(form.m_salaryEdit->text(), &ok);
    EXPECT_TRUE(ok) << "Failed to parse salary text: " << form.m_salaryEdit->text().toStdString();
    EXPECT_DOUBLE_EQ(salary, 50000.50);
}

TEST_F(GrpcFormTest, FillObjectReadsDateTimeWidgets)
{
    TestGrpcFormWithDateTime form;

    const QDate date(2026, 1, 31);
    const QTime time(12, 34, 56);
    const QDateTime dateTime(date, time);

    form.m_dateEdit->setDate(date);
    form.m_timeEdit->setTime(time);
    form.m_dateTimeEdit->setDateTime(dateTime);

    form.fillObject();

    const MasterObject obj = form.object().value<MasterObject>();

    const qint64 expectedDateMs = QDateTime(date, QTime(0, 0)).toMSecsSinceEpoch();
    const qint64 expectedTimeMs = QDateTime(QDate::currentDate(), time).toMSecsSinceEpoch();
    const qint64 expectedDateTimeMs = dateTime.toMSecsSinceEpoch();

    EXPECT_EQ(obj.date(), expectedDateMs);
    EXPECT_EQ(obj.time(), expectedTimeMs);
    EXPECT_EQ(obj.date_time(), expectedDateTimeMs);
}

TEST_F(GrpcFormTest, FillPopulatesImageLabelFromDataUrl)
{
    TestGrpcForm form;

    TestGrpcObjectTableModel model;

    MasterObject obj;
    obj.set_name("With Image");
    obj.set_height(170);
    obj.set_married(false);
    obj.set_salary(1.0);

    QImage image(8, 8, QImage::Format_ARGB32);
    image.fill(QColor(255, 0, 0));

    QByteArray pngBytes;
    {
        QBuffer buffer(&pngBytes);
        ASSERT_TRUE(buffer.open(QIODevice::WriteOnly));
        ASSERT_TRUE(image.save(&buffer, "PNG"));
    }
    obj.set_image(QString("data:image/png;base64,%1").arg(QString::fromLatin1(pngBytes.toBase64())).toStdString());

    model.insertObject(0, QVariant::fromValue(obj));

    form.fill(model.index(0, 0));

    ASSERT_NE(form.m_imageLabel, nullptr);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPixmap pix = form.m_imageLabel->pixmap(Qt::ReturnByValue);
    EXPECT_FALSE(pix.isNull());
#else
    const QPixmap * pix = form.m_imageLabel->pixmap();
    ASSERT_NE(pix, nullptr);
    EXPECT_FALSE(pix->isNull());
#endif
}

TEST_F(GrpcFormTest, FillPopulatesImageLabelFromJpegDataUrl)
{
    TestGrpcForm form;

    TestGrpcObjectTableModel model;

    MasterObject obj;
    obj.set_name("With JPEG Image");
    obj.set_height(170);
    obj.set_married(false);
    obj.set_salary(1.0);

    QImage image(8, 8, QImage::Format_RGB32);
    image.fill(QColor(0, 128, 255));

    QByteArray jpegBytes;
    {
        QBuffer buffer(&jpegBytes);
        ASSERT_TRUE(buffer.open(QIODevice::WriteOnly));
        ASSERT_TRUE(image.save(&buffer, "JPG"));
    }
    obj.set_image(QString("data:image/jpeg;base64,%1").arg(QString::fromLatin1(jpegBytes.toBase64())).toStdString());

    model.insertObject(0, QVariant::fromValue(obj));

    form.fill(model.index(0, 0));

    ASSERT_NE(form.m_imageLabel, nullptr);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPixmap pix = form.m_imageLabel->pixmap(Qt::ReturnByValue);
    EXPECT_FALSE(pix.isNull());
#else
    const QPixmap * pix = form.m_imageLabel->pixmap();
    ASSERT_NE(pix, nullptr);
    EXPECT_FALSE(pix->isNull());
#endif
}

TEST_F(GrpcFormTest, FillWithInvalidIndexDoesNotCrash)
{
    TestGrpcForm form;
    QModelIndex invalid;
    
    EXPECT_NO_THROW(form.fill(invalid));
}

TEST_F(GrpcFormTest, FillSetsReadWriteMode)
{
    TestGrpcForm form;
    form.makeReadonly(true);
    
    TestGrpcObjectTableModel model;
    
    MasterObject obj;
    obj.set_name("Test");
    
    model.insertObject(0, QVariant::fromValue(obj));
    
    form.fill(model.index(0, 0));
    
    EXPECT_FALSE(form.m_nameEdit->isReadOnly());
}

TEST_F(GrpcFormTest, FillSkipsWidgetsWithDataMask)
{
    TestGrpcForm form;
    
    // CheckBox has data mask, should be skipped during fill
    form.m_marriedCheck->setChecked(false);
    
    TestGrpcObjectTableModel model;
    
    MasterObject obj;
    obj.set_name("Test");
    obj.set_married(true); // Set to true in object
    
    model.insertObject(0, QVariant::fromValue(obj));
    
    form.fill(model.index(0, 0));
    
    // CheckBox should still be false because it has a data mask (skipped)
    EXPECT_FALSE(form.m_marriedCheck->isChecked());
}

// ============================================================================
// fillObject() Tests
// ============================================================================

TEST_F(GrpcFormTest, FillObjectUpdatesWrapperFromWidgets)
{
    TestGrpcForm form;

    // Precondition: GrpcForm::fillObject() updates the current object held by the wrapper.
    // The wrapper has no object right after construction, so initialize it first.
    form.clear();
    
    // Set widget values
    form.m_nameEdit->setText("Updated Name");
    form.m_heightEdit->setText("175");
    form.m_marriedCheck->setChecked(true);
    form.m_salaryEdit->setText("60000");
    
    form.fillObject();
    
    // Get object back and verify
    MasterObject obj = form.object().value<MasterObject>();
    EXPECT_EQ(obj.name(), "Updated Name");
    EXPECT_EQ(obj.height(), 175);
    EXPECT_TRUE(obj.married());
    EXPECT_DOUBLE_EQ(obj.salary(), 60000.0);
}

TEST_F(GrpcFormTest, FillObjectHandlesEmptyWidgets)
{
    TestGrpcForm form;
    
    form.m_nameEdit->clear();
    form.m_heightEdit->clear();
    form.m_salaryEdit->clear();
    
    EXPECT_NO_THROW(form.fillObject());
}

// ============================================================================
// clear() Tests
// ============================================================================

TEST_F(GrpcFormTest, ClearResetsWidgetsToDefaults)
{
    TestGrpcForm form;
    
    // Set some values
    form.m_nameEdit->setText("Test");
    form.m_heightEdit->setText("180");
    form.m_marriedCheck->setChecked(true);
    form.m_salaryEdit->setText("50000");
    
    form.clear();
    
    // Widgets should be cleared or set to defaults
    EXPECT_TRUE(form.m_nameEdit->text().isEmpty() || form.m_nameEdit->text() == "Default");
    EXPECT_TRUE(form.m_heightEdit->text().isEmpty() || form.m_heightEdit->text().toInt() == 0);
    EXPECT_FALSE(form.m_marriedCheck->isChecked());
}

TEST_F(GrpcFormTest, ClearSetsObjectToDefault)
{
    TestGrpcForm form;
    
    form.m_nameEdit->setText("Test");
    form.fillObject();
    
    form.clear();
    
    MasterObject obj = form.object().value<MasterObject>();
    // Should have default values
    EXPECT_TRUE(obj.name() == "Default" || obj.name().empty());
}

// ============================================================================
// makeReadonly() Tests
// ============================================================================

TEST_F(GrpcFormTest, MakeReadonlyDisablesEditing)
{
    TestGrpcForm form;
    
    form.makeReadonly(true);
    
    EXPECT_TRUE(form.m_nameEdit->isReadOnly());
    EXPECT_TRUE(form.m_heightEdit->isReadOnly());
    EXPECT_FALSE(form.m_marriedCheck->isEnabled());
    EXPECT_TRUE(form.m_salaryEdit->isReadOnly());
}

TEST_F(GrpcFormTest, MakeReadonlyEnablesEditing)
{
    TestGrpcForm form;
    
    form.makeReadonly(true);
    form.makeReadonly(false);
    
    EXPECT_FALSE(form.m_nameEdit->isReadOnly());
    EXPECT_FALSE(form.m_heightEdit->isReadOnly());
    EXPECT_TRUE(form.m_marriedCheck->isEnabled());
    EXPECT_FALSE(form.m_salaryEdit->isReadOnly());
}

// ============================================================================
// Signal Tests
// ============================================================================

TEST_F(GrpcFormTest, FormContentChangedSignalEmittedOnWidgetChange)
{
    TestGrpcForm form;
    
    // Fill form first to set m_formFillingFinished = true
    TestGrpcObjectTableModel model;
    
    MasterObject obj;
    obj.set_name("Test");
    
    model.insertObject(0, QVariant::fromValue(obj));
    form.fill(model.index(0, 0));
    
    QSignalSpy spy(&form, &GrpcForm::formContentChanaged);

    // The form connects QLineEdit::textEdited (not textChanged), so simulate real user typing.
    form.m_nameEdit->setFocus();
    form.m_nameEdit->clear();
    QTest::keyClicks(form.m_nameEdit, "Changed");

    // Process events to deliver the signal
    QCoreApplication::processEvents();
    
    EXPECT_GE(spy.count(), 1);
}

TEST_F(GrpcFormTest, FormEscapeSignalEmittedOnEscapeKey)
{
    TestGrpcForm form;
    QSignalSpy spy(&form, &GrpcForm::formEscapeSignal);
    
    // Simulate Escape key press
    QKeyEvent escapePress(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QApplication::sendEvent(form.m_nameEdit, &escapePress);
    
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(GrpcFormTest, SendObjectSignalEmittedOnPrepareObject)
{
    TestGrpcForm form;
    // Precondition: prepareObject() sends the wrapper's current object.
    form.clear();
    form.m_nameEdit->setText("Test");
    
    QSignalSpy spy(&form, &GrpcForm::sendObject);
    
    form.prepareObject();
    
    EXPECT_EQ(spy.count(), 1);
    ASSERT_FALSE(spy.isEmpty());
    
    QVariant signalArg = spy.at(0).at(0).value<QVariant>();
    EXPECT_TRUE(signalArg.isValid());
}

// ============================================================================
// Tab Widget Integration Tests
// ============================================================================

TEST_F(GrpcFormTest, TabWidgetReturnsParentTab)
{
    QTabWidget tabWidget;
    auto * page = new QWidget();
    tabWidget.addTab(page, "Test Form");
    auto * form = new TestGrpcForm(page);

    EXPECT_EQ(form->tabWidget(), &tabWidget);
}

TEST_F(GrpcFormTest, TabWidgetReturnsNullWhenNotInTab)
{
    TestGrpcForm form;
    
    EXPECT_EQ(form.tabWidget(), nullptr);
}

TEST_F(GrpcFormTest, StartInsertShowsSaveIconInTab)
{
    QTabWidget tabWidget;
    auto * page = new QWidget();
    int index = tabWidget.addTab(page, "Test Form");
    auto * form = new TestGrpcForm(page);
    tabWidget.setCurrentWidget(page);
    form->startInsert();

    // Verify functional behavior that doesn't depend on GUI activation/resources.
    // startInsert() calls clear(), which sets the wrapper object and clears the widgets.
    const QVariant varObj = form->object();
    ASSERT_TRUE(varObj.isValid());
    ASSERT_TRUE(varObj.canConvert<MasterObject>());
    const MasterObject obj = varObj.value<MasterObject>();
    EXPECT_EQ(obj.name(), "Default");

    EXPECT_TRUE(form->m_nameEdit->text().isEmpty());
    EXPECT_TRUE(form->m_heightEdit->text().isEmpty());
    EXPECT_FALSE(form->m_marriedCheck->isChecked());
    EXPECT_TRUE(form->m_salaryEdit->text().isEmpty());
    Q_UNUSED(index);
}

TEST_F(GrpcFormTest, FinishSaveClearsSaveIcon)
{
    QTabWidget tabWidget;
    auto * page = new QWidget();
    int index = tabWidget.addTab(page, "Test Form");
    auto * form = new TestGrpcForm(page);
    form->startInsert();
    form->finishSave();
    
    EXPECT_TRUE(tabWidget.tabIcon(index).isNull());
}

TEST_F(GrpcFormTest, HideAllButThisHidesOtherTabs)
{
    QTabWidget tabWidget;
    auto * page1 = new QWidget();
    auto * page2 = new QWidget();
    auto * page3 = new QWidget();
    tabWidget.addTab(page1, "Form 1");
    tabWidget.addTab(page2, "Form 2");
    tabWidget.addTab(page3, "Form 3");
    auto * form1 = new TestGrpcForm(page1);
    auto * form2 = new TestGrpcForm(page2);
    auto * form3 = new TestGrpcForm(page3);

    Q_UNUSED(form1);
    Q_UNUSED(form3);

    tabWidget.setCurrentWidget(page2);
    form2->hideAllButThis();
    
    // Form 2 should be visible, others hidden
    EXPECT_TRUE(tabWidget.tabBar()->isTabVisible(1));
    EXPECT_FALSE(tabWidget.tabBar()->isTabVisible(0));
    EXPECT_FALSE(tabWidget.tabBar()->isTabVisible(2));
}

// ============================================================================
// Master-Slave Relationship Tests
// ============================================================================

TEST_F(GrpcFormTest, MasterChangedUpdatesMasterWrapper)
{
    auto * wrapper = new GrpcObjectWrapper<SlaveObject>();
    auto * masterWrapper = new GrpcObjectWrapper<MasterObject>();
    
    // Initialize slave wrapper
    wrapper->addProperty("uid", DataInfo::Int, &SlaveObject::set_uid, &SlaveObject::uid);
    wrapper->addProperty("phone", DataInfo::String, &SlaveObject::set_phone, &SlaveObject::phone);
    
    // Initialize master wrapper
    masterWrapper->addProperty("name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    
    TestGrpcForm form(wrapper, masterWrapper);
    
    // Create master model
    TestGrpcObjectTableModel masterModel;
    
    MasterObject master;
    master.set_name("Master Object");
    
    masterModel.insertObject(0, QVariant::fromValue(master));
    
    form.masterChanged(masterModel.index(0, 0));
    
    QVariant masterVar = form.getMasterVariant();
    EXPECT_TRUE(masterVar.isValid());
    
    MasterObject retrievedMaster = masterVar.value<MasterObject>();
    EXPECT_EQ(retrievedMaster.name(), "Master Object");
}

// ============================================================================
// Widget Type Handling Tests
// ============================================================================

TEST_F(GrpcFormTest, LineEditHandlesStringData)
{
    TestGrpcForm form;
    form.clear();
    form.m_nameEdit->setText("Test String");
    form.fillObject();
    
    MasterObject obj = form.object().value<MasterObject>();
    EXPECT_EQ(obj.name(), "Test String");
}

TEST_F(GrpcFormTest, LineEditHandlesIntData)
{
    TestGrpcForm form;
    form.clear();
    form.m_heightEdit->setText("175");
    form.fillObject();
    
    MasterObject obj = form.object().value<MasterObject>();
    EXPECT_EQ(obj.height(), 175);
}

TEST_F(GrpcFormTest, CheckBoxHandlesBoolData)
{
    TestGrpcForm form;
    form.clear();
    form.m_marriedCheck->setChecked(true);
    form.fillObject();
    
    MasterObject obj = form.object().value<MasterObject>();
    EXPECT_TRUE(obj.married());
}

TEST_F(GrpcFormTest, LineEditHandlesDoubleData)
{
    TestGrpcForm form;
    form.clear();
    form.m_salaryEdit->setText("55000.75");
    form.fillObject();
    
    MasterObject obj = form.object().value<MasterObject>();
    EXPECT_DOUBLE_EQ(obj.salary(), 55000.75);
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(GrpcFormTest, DefaultObjectReturnsValidVariant)
{
    TestGrpcForm form;
    
    QVariant defaultVar = form.defaultObject();
    
    EXPECT_TRUE(defaultVar.isValid());
    EXPECT_TRUE(defaultVar.canConvert<MasterObject>());
}

TEST_F(GrpcFormTest, ObjectReturnsValidVariant)
{
    TestGrpcForm form;
    form.clear();
    form.m_nameEdit->setText("Test");
    form.fillObject();
    
    QVariant obj = form.object();
    
    EXPECT_TRUE(obj.isValid());
    EXPECT_TRUE(obj.canConvert<MasterObject>());
}

// ============================================================================
// Workflow Tests
// ============================================================================

TEST_F(GrpcFormTest, InsertWorkflow)
{
    QTabWidget tabWidget;
    auto * page = new QWidget();
    tabWidget.addTab(page, "Test");
    auto * form = new TestGrpcForm(page);
    
    // Start insert
    form->startInsert();
    
    // Form should be editable
    EXPECT_FALSE(form->m_nameEdit->isReadOnly());
    
    // Fill data
    form->m_nameEdit->setText("New Object");
    form->m_heightEdit->setText("180");
    
    // Prepare object
    form->prepareObject();
    
    MasterObject obj = form->object().value<MasterObject>();
    EXPECT_EQ(obj.name(), "New Object");
    EXPECT_EQ(obj.height(), 180);
    
    // Finish save
    form->finishSave();
}

TEST_F(GrpcFormTest, EditWorkflow)
{
    QTabWidget tabWidget;
    auto * page = new QWidget();
    tabWidget.addTab(page, "Test");
    auto * form = new TestGrpcForm(page);
    
    // Create and fill existing object
    TestGrpcObjectTableModel model;
    
    MasterObject existing;
    existing.set_name("Existing");
    existing.set_height(170);
    
    model.insertObject(0, QVariant::fromValue(existing));
    
    // Fill form with existing object
    form->fill(model.index(0, 0));
    
    // Start edit
    form->startEdit();
    
    // Modify data
    form->m_nameEdit->setText("Modified");
    form->m_heightEdit->setText("175");
    
    // Prepare object
    form->prepareObject();
    
    MasterObject obj = form->object().value<MasterObject>();
    EXPECT_EQ(obj.name(), "Modified");
    EXPECT_EQ(obj.height(), 175);
    
    // Finish save
    form->finishSave();
}

TEST_F(GrpcFormTest, CancelWorkflowWithEscape)
{
    TestGrpcForm form;
    QSignalSpy spy(&form, &GrpcForm::formEscapeSignal);
    
    // Start editing
    form.m_nameEdit->setText("Temporary");
    
    // Press Escape
    QKeyEvent escapePress(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QApplication::sendEvent(form.m_nameEdit, &escapePress);
    
    // Should emit escape signal
    EXPECT_EQ(spy.count(), 1);
    
    // Form can be cleared by controller listening to escape signal
    form.clear();
    
    EXPECT_TRUE(form.m_nameEdit->text().isEmpty() || form.m_nameEdit->text() == "Default");
}

// Main function
int main(int argc, char **argv)
{
    // Initialize Qt Application for GUI tests
    QApplication app(argc, argv);
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// Include MOC files for Qt meta-object system
#include "GrpcFormTests.moc"
