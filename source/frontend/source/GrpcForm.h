#ifndef GRPCFORM_H
#define GRPCFORM_H

#include <QWidget>
#include <QTabBar>
#include <QTabWidget>

#include "GrpcObjectWrapper.hpp"

class QEvent;

/**
 * @brief Base class for GRPC object forms with automatic widget binding
 * 
 * GrpcForm provides a generic form framework for editing GRPC objects with automatic
 * widget binding, validation, and change tracking. It supports various Qt widgets:
 * - QLineEdit (with validators for numbers, strings)
 * - QComboBox
 * - QCheckBox
 * - QDateEdit, QTimeEdit, QDateTimeEdit
 * - QTextEdit
 * 
 * Features:
 * - Automatic bi-directional data binding between widgets and GRPC objects
 * - Built-in input validation (SQL injection prevention, number validation)
 * - Change tracking with signals
 * - Read-only mode support
 * - Tab widget integration with save indicators
 * - Master-slave form relationships
 * 
 * Usage:
 * 1. Derive from GrpcForm
 * 2. Override initializeForm() to set up widgets
 * 3. Override defaultObject() to provide default values
 * 4. Create widgets with names matching property names
 * 
 * @note Widget names must match property names from GrpcObjectWrapper
 * @note Thread-safety: All operations must be on the GUI thread
 */
class GrpcForm : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a GrpcForm
     * @param objectWrapper Wrapper for the main object (ownership transferred)
     * @param masterObjectWrapper Wrapper for master object in master-slave relationship (ownership transferred, can be nullptr)
     * @param parent Parent widget
     */
    explicit GrpcForm(IBaseGrpcObjectWrapper * objectWrapper, IBaseGrpcObjectWrapper * masterObjectWrapper, QWidget * parent = nullptr);
    /**
     * @brief Destructor
     */
    virtual ~GrpcForm();

    /**
     * @brief Returns the current object as QVariant
     * @return QVariant containing the GRPC object
     */
    QVariant object();
    
    /**
     * @brief Returns the parent tab widget if form is in a tab
     * @return Pointer to QTabWidget or nullptr if not in a tab
     */
    QTabWidget * tabWidget();

public slots:
    /**
     * @brief Fills form with object data from model index
     * @param index Model index containing object data in VariantObjectRole
     * 
     * @note Can be overridden in derived classes
     * @note Sets form to read-write mode
     * @note Skips widgets with data masks (ComboBox, CheckBox)
     */
    virtual void fill(const QModelIndex & index);
    
    /**
     * @brief Clears all form widgets to default values
     * 
     * @note Sets combo boxes to -1, line edits to empty, dates to current
     * @note Can be overridden in derived classes
     */
    virtual void clear();
    
    /**
     * @brief Sets form to read-only or read-write mode
     * @param readOnly True for read-only, false for editable
     * 
     * @note Can be overridden in derived classes
     */
    virtual void makeReadonly(bool readOnly);
    
    /**
     * @brief Reads widget values back into the wrapped object
     * 
     * @note Handles ComboBox and CheckBox data masks specially
     * @note Can be overridden in derived classes
     */
    virtual void fillObject();
    
    /**
     * @brief Updates master object when master selection changes
     * @param index Model index of selected master object
     * 
     * @note Only used in slave forms of master-slave relationships
     * @note Can be overridden in derived classes
     */
    virtual void masterChanged(const QModelIndex & index);

    /**
     * @brief Hides all tabs except the one containing this form
     * 
     * @note Used to focus user attention on current form
     */
    void hideAllButThis();
    
    /**
     * @brief Fills object from widgets and emits sendObject signal
     * 
     * @note Convenience method combining fillObject() and object()
     */
    void prepareObject();

    /**
     * @brief Prepares form for inserting a new object
     * 
     * @note Clears form, shows save icon, sets focus to first widget
     */
    void startInsert();
    
    /**
     * @brief Prepares form for editing existing object
     * 
     * @note Shows save icon in tab
     */
    void startEdit();
    
    /**
     * @brief Clears save indicator after successful save
     * 
     * @note Removes save icon from tab
     */
    void finishSave();

protected:
    friend class GrpcTemplateController;
    friend class TestGrpcForm;  // For testing
    
    /**
     * @brief Pure virtual method to initialize form widgets
     * 
     * Derived classes must override this to:
     * 1. Create and configure widgets
     * 2. Add properties to object wrapper
     * 3. Call initilizeWidgets() to bind widgets
     * 
     * @note Must be implemented by derived classes
     */
    virtual void initializeForm() = 0;
    
    /**
     * @brief Pure virtual method to provide default object
     * @return QVariant containing default GRPC object
     * 
     * @note Must be implemented by derived classes
     * @note Used when clearing form or creating new objects
     */
    virtual QVariant defaultObject() = 0;
    
    /**
     * @brief Returns pointer to the object wrapper
     * @return Pointer to IBaseGrpcObjectWrapper
     */
    IBaseGrpcObjectWrapper * objectWrapper() {return m_objectWrapper.get();}

    /**
     * @brief Event filter to capture Escape key for cancellation
     * @param watched The watched object
     * @param event The event
     * @return true if event handled (Escape key), false otherwise
     */
    bool eventFilter(QObject *watched, QEvent * event) override;
    
    /**
     * @brief Returns the master object as QVariant
     * @return QVariant containing master object, or invalid if no master
     */
    QVariant masterVariantObject();

signals:
    /**
     * @brief Emitted when form content changes
     * 
     * @note Only emitted after form filling is complete (not during fill())
     */
    void formContentChanaged();
    
    /**
     * @brief Emitted when user presses Escape key
     * 
     * @note Used to cancel current operation
     */
    void formEscapeSignal();
    
    /**
     * @brief Emitted when object is ready to be sent
     * @param object The GRPC object as QVariant
     */
    void sendObject(const QVariant & object);

private slots:
    /**
     * @brief Internal slot called when widget content changes
     * 
     * @note Only forwards signal if form filling is complete
     */
    void contentChanged();

private:
    /**
     * @brief Fills a single widget with data
     * @param widget Widget to fill
     * @param type Data type
     * @param data Data value
     * 
     * @note Handles type-specific formatting and validation
     */
    void fillWidget(QWidget * widget, const DataInfo::Type & type, const QVariant & data);
    
    /**
     * @brief Extracts data from a widget
     * @param widget Widget to read from
     * @param type Expected data type
     * @return QVariant containing widget data
     * 
     * @note Handles type-specific extraction and conversion
     */
    QVariant widgetData(QWidget * widget, const DataInfo::Type & type);
    
    /**
     * @brief Initializes and connects all form widgets
     * 
     * @note Sets up validators, event filters, and change signals
     * @note Must be called after widgets are created
     */
    void initilizeWidgets();
    
    /**
     * @brief Returns the tab bar containing this form
     * @return Pointer to QTabBar or nullptr
     */
    QTabBar * tabBar();
    
    /**
     * @brief Returns the tab index of this form
     * @return Tab index or -1 if not in a tab
     */
    int tabIndex();

    /** @brief List of managed form widgets (in property order) */
    QList<QWidget*> m_formWidgets;
    
    /** @brief Wrapper for the main GRPC object */
    std::unique_ptr<IBaseGrpcObjectWrapper> m_objectWrapper = nullptr;
    
    /** @brief Wrapper for master object in master-slave relationship */
    std::unique_ptr<IBaseGrpcObjectWrapper> m_masterObjectWrapper = nullptr;
    
    /** @brief Icon shown in tab during unsaved changes */
    QIcon m_saveIcon;

    /** @brief Flag indicating form filling is complete */
    bool m_formFillingFinished = false;
    
    /** @brief Flag indicating form is in read-only mode */
    bool m_readonly = false;
};

#endif // GRPCFORM_H
