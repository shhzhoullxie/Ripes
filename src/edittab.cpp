#include "edittab.h"
#include "ui_edittab.h"

#include "parser.h"
#include "pipeline.h"

EditTab::EditTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::EditTab) {
    m_ui->setupUi(this);

    // Only add syntax highlighter for code edit view - not for translated code. This is assumed to be correct after a
    // translation is complete
    m_ui->assemblyedit->setupSyntaxHighlighter();
    m_ui->assemblyedit->setupAssembler();
    m_ui->binaryedit->setReadOnly(true);
    // enable breakpoint area for the translated code only
    m_ui->binaryedit->enableBreakpointArea();

    // Link scrollbars together for pleasant navigation
    connect(m_ui->assemblyedit->verticalScrollBar(), &QScrollBar::valueChanged, m_ui->binaryedit->verticalScrollBar(),
            &QScrollBar::setValue);
    connect(m_ui->binaryedit->verticalScrollBar(), &QScrollBar::valueChanged, m_ui->assemblyedit->verticalScrollBar(),
            &QScrollBar::setValue);

    // Connect data parsing signals from the assembler to this
    connect(m_ui->assemblyedit, &CodeEditor::assembledSuccessfully, this, &EditTab::assemblingComplete);
}

QString EditTab::getAssemblyText() {
    return m_ui->assemblyedit->toPlainText();
}

const QByteArray& EditTab::getBinaryData() {
    return m_ui->assemblyedit->getCurrentOutputArray();
}

void EditTab::clearOutputArray() {
    m_ui->assemblyedit->clearOutputArray();
}

EditTab::~EditTab() {
    delete m_ui;
}

void EditTab::setTimerEnabled(bool state) {
    m_ui->assemblyedit->setTimerEnabled(state);
}

void EditTab::newProgram() {
    m_ui->assemblyedit->reset();
    m_ui->assemblyedit->clear();
}

void EditTab::setAssemblyText(const QString& text) {
    m_ui->assemblyedit->reset();
    m_ui->assemblyedit->setPlainText(text);
}

void EditTab::setDisassemblerText() {
    const QString& text = m_ui->disassembledViewButton->isChecked() ? Parser::getParser()->getDisassembledRepr()
                                                                    : Parser::getParser()->getBinaryRepr();
    m_ui->binaryedit->setPlainText(text);
    m_ui->binaryedit->updateBreakpoints();
}

void EditTab::assemblingComplete(const QByteArray& arr, bool clear, uint32_t baseAddress) {
    if (clear)
        Parser::getParser()->clear();
    // Pretty hacky way to discern between the text and data segments
    if (baseAddress > 0) {
        Parser::getParser()->loadFromByteArrayIntoData(arr);
    } else {
        Parser::getParser()->loadFromByteArray(arr, m_ui->disassembledViewButton->isChecked(), baseAddress);
        setDisassemblerText();
    }
    emit updateSimulator();
}

void EditTab::on_assemblyfile_toggled(bool checked) {
    // Since we are removing the input text/binary info, we need to reset the pipeline
    Pipeline::getPipeline()->reset();

    // handles toggling between assembly input and binary input
    if (checked) {
        m_ui->assemblyedit->setEnabled(true);
        m_ui->assemblyedit->setTimerEnabled(true);
    } else {
        // Disable when loading binary files
        m_ui->assemblyedit->setTimerEnabled(false);
        m_ui->assemblyedit->setEnabled(false);
    }
    // clear both editors when switching input mode and reset the highlighter for the assembly editor
    m_ui->assemblyedit->clear();
    m_ui->assemblyedit->reset();
    m_ui->binaryedit->clear();
}

void EditTab::setInputMode(bool isAssembly) {
    /*
    if (isAssembly) {
        m_ui->assemblyfile->setChecked(true);
    } else {
        m_ui->binaryfile->setChecked(true);
    }
    */
}

void EditTab::on_disassembledViewButton_toggled(bool checked) {
    Q_UNUSED(checked)
    // if (m_ui->binaryfile->isChecked()) {
    //    assemblingComplete(Parser::getParser()->getFileByteArray());
    //} else {
    assemblingComplete(m_ui->assemblyedit->getCurrentOutputArray());
    // }
}