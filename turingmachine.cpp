#include "turingmachine.h"
#include "ui_turingmachine.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QMainWindow>

TuringMachine::TuringMachine(const QString& Alphabet,
                              const QString& AddAlphabet,
                              QWidget* parent):
    QWidget(parent),
    ui(new Ui::TuringMachine),
    m_alphabet(Alphabet),
    m_addAlphabet(AddAlphabet)
{
    ui->setupUi(this);

    for (QChar ch : m_alphabet) {
        if (!ch.isSpace()) {
            m_allowed.insert(ch);
        }
    }
    for (QChar ch : m_addAlphabet) {
        if (!ch.isSpace()) {
            m_allowed.insert(ch);
        }
    }

    CreateTable();
}

TuringMachine::~TuringMachine()
{
    delete ui;
}

void TuringMachine::CreateTable() {
    QStringList main_symbols;
    for (QChar ch : m_alphabet) {
        if (!ch.isSpace()) {
            main_symbols << ch;
        }
    }
    main_symbols.removeDuplicates();
    QStringList add_symbols;
    for (QChar ch : m_addAlphabet) {
        if (!ch.isSpace()) {
            add_symbols << ch;
        }
    }

    add_symbols.removeDuplicates();

    QString blank = "λ";

    QStringList symbols;
    symbols << main_symbols;
    if (!symbols.contains(blank)) {
        symbols.append(blank);
    } else {
        symbols.removeAll(blank);
        symbols.append(blank);
    }
    symbols << add_symbols;

    symbols.removeDuplicates();

    m_allSymbols = symbols;


    QTableWidget* tw = ui->table;
    tw->clear();
    tw->setColumnCount(1 + m_allSymbols.size());
    tw->setRowCount(1);

    QStringList headers;
    headers << "Состояние";
    headers.append(m_allSymbols);
    tw->setHorizontalHeaderLabels(headers);

    tw->setItem(0, 0, new QTableWidgetItem("q0"));

    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tw->verticalHeader()->setVisible(false);
}

void TuringMachine::CreateTape(const QString& input) {
    const int padding = VISIBLE_COLS / 2;
    QString blank = "λ";

    m_tapeSymbols.clear();
    for (int i = 0; i < padding; ++i) {
        m_tapeSymbols << blank;
    }
    for (QChar ch : input) {
        m_tapeSymbols << QString(ch);
    }
    for (int i = 0; i < padding; ++i) {
        m_tapeSymbols << blank;
    }

    QTableWidget* tw = ui->tape;
    tw->clear();
    tw->setRowCount(1);
    tw->setColumnCount(VISIBLE_COLS);

    tw->horizontalHeader()->setVisible(false);
    tw->verticalHeader()->setVisible(false);\
    tw->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tw->setSelectionMode(QAbstractItemView::NoSelection);
    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tw->verticalHeader()->setDefaultSectionSize(40);
    tw->setFixedHeight(42);

    m_index = padding;

    UpdateView();
}

void TuringMachine::UpdateView() {
    const int half = VISIBLE_COLS / 2;
    QString blank = "λ";
    QTableWidget* tw = ui->tape;

    while (m_index - half < 0) {
        m_tapeSymbols.prepend(blank);
        ++m_index;
    }
    while (m_index + half >= m_tapeSymbols.size()) {
        m_tapeSymbols.append(blank);
    }

    for (int col = 0; col < VISIBLE_COLS; ++col) {
        int tapePos = m_index - half + col;
        QTableWidgetItem* item = new QTableWidgetItem(m_tapeSymbols[tapePos]);
        item->setTextAlignment(Qt::AlignCenter);

        if (col == half) {
            item->setBackground(QBrush(QColor(0xE0, 0xF0, 0xFF)));
        } else {
            item->setBackground(QBrush(QColor(Qt::white)));
        }
        tw->setItem(0, col, item);
    }
}

void TuringMachine::on_add_state_clicked() {
    QTableWidget* tw = ui->table;
    int new_row = tw->rowCount();
    tw->insertRow(new_row);
    tw->setItem(new_row, 0, new QTableWidgetItem(QString("q%1").arg(++m_nextState)));
}

void TuringMachine::on_del_state_clicked() {
    QTableWidget* tw = ui->table;
    int last_row = tw->rowCount() - 1;
    if (last_row > 0) {
        tw->removeRow(last_row);
        --m_nextState;
    } else {
        QMessageBox::information(this, "Ошибка", "Нельзя удалить единственное состояние.");
    }
}

void TuringMachine::on_confirm_str_clicked() {
    QString input = ui->w_string->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::information(this, "Запрет", "Строка не может быть пустой.");
        return;
    }

    for (QChar ch : input) {
        if (!m_allowed.contains(ch)) {
            QMessageBox::information(this, "Запрет", QString("Обнаружен недопустимый символ: '%1'").arg(ch));
            return;
        }
    }
    CreateTape(input);
}
