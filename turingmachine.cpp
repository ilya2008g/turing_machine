#include "turingmachine.h"
#include "ui_turingmachine.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QMainWindow>
#include <QTimer>

TuringMachine::TuringMachine(const QString& Alphabet,
                              const QString& AddAlphabet,
                              QWidget* parent):
    QWidget(parent),
    ui(new Ui::TuringMachine),
    m_alphabet(Alphabet),
    m_addAlphabet(AddAlphabet)
{
    ui->setupUi(this);

    setWindowTitle("turing_machine");
    this->setFixedSize(700, 500);

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

    m_timerInterval = 500;
    m_timer = new QTimer(this);
    m_timer->setInterval(m_timerInterval);
    connect(m_timer, &QTimer::timeout, this, &TuringMachine::executeStep);

    ui->pause->setEnabled(false);
    ui->stop->setEnabled(false);

    UpdateSpeedDisplay();
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
    if (symbols.contains(blank)) {
        symbols.removeAll(blank);
    }
    symbols.append(blank);
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
    tw->verticalHeader()->setVisible(false);
    tw->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tw->setSelectionMode(QAbstractItemView::NoSelection);
    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tw->verticalHeader()->setDefaultSectionSize(40);
    tw->setFixedHeight(42);

    m_index = padding;
    m_viewOffset = qMax(0, m_index - VISIBLE_COLS / 2);

    UpdateView();
    QTimer::singleShot(0, this, [this] {
        UpdateHead();
    });

    m_initInput = input;
    m_initIndex = padding;
}

void TuringMachine::UpdateView() {
    QString blank = "λ";
    QTableWidget* tw = ui->tape;

    while (m_index < 0) {
        m_tapeSymbols.prepend(blank);
        ++m_index;
        ++m_viewOffset;
    }
    while (m_index >= m_tapeSymbols.size()) {
        m_tapeSymbols.append(blank);
    }

    for (int col = 0; col < VISIBLE_COLS; ++col) {
        int tapePos = m_viewOffset + col;
        QTableWidgetItem* item = new QTableWidgetItem(m_tapeSymbols[tapePos]);
        item->setTextAlignment(Qt::AlignCenter);

        if (tapePos == m_index) {
            item->setBackground(QBrush(QColor(0xE0, 0xF0, 0xFF)));
        } else {
            item->setBackground(QBrush(QColor(Qt::white)));
        }
        tw->setItem(0, col, item);
    }
}

void TuringMachine::UpdateHead() {
    if (!ui->head || !ui->tape) return;

    int col = m_index - m_viewOffset;
    if (col < 0 || col >= VISIBLE_COLS) return;

    QTableWidget* tw = ui->tape;
    QRect cell = tw->visualRect(tw->model()->index(0, col));
    QPoint top = tw->viewport()->mapTo(tw->parentWidget(), cell.topLeft());

    int x = top.x() + cell.width() / 2 - ui->head->width() / 2 - 5;
    int y = top.y() + ui->head->height() + 20;

    if (m_headAnimation && m_headAnimation->state() == QAbstractAnimation::Running) {
        m_headAnimation->stop();
    }

    QPropertyAnimation* anim = new QPropertyAnimation(ui->head, "pos");
    anim->setDuration(100);
    anim->setStartValue(ui->head->pos());
    anim->setEndValue(QPoint(x, y));
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    ui->head->raise();
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

void TuringMachine::on_play_clicked() {
    if (!m_simInit) {
        QString error = validateAndLoadRules();
        if (!error.isEmpty()) {
            QMessageBox::warning(this, "Обнаружены ошибки", error);
            return;
        }
        if (m_tapeSymbols.isEmpty()) {
            QMessageBox::warning(this, "Пустая строка", "Сначала введите строку");
            return;
        }
        StartSimulation();
    }

    if (!m_timerActive) {
        m_timer->start(m_timerInterval);
        m_timerActive = true;
        ui->play->setEnabled(false);
        ui->pause->setEnabled(true);
    }
}

void TuringMachine::on_step_clicked() {
    if (m_timerActive) {
        stopTimer();
    }
    if (!m_simInit) {
        QString error = validateAndLoadRules();
        if (!error.isEmpty()) {
            QMessageBox::warning(this, "Обнаружены ошибки", error);
            return;
        }
        if (m_tapeSymbols.isEmpty()) {
            QMessageBox::warning(this, "Пустая строка", "Сначала введите строку");
            return;
        }
        StartSimulation();
    }
    executeStep();
}

void TuringMachine::on_pause_clicked() {
    if (m_timerActive) {
        stopTimer();
    }
}

void TuringMachine::on_stop_clicked() {
    stopTimer();
    if (m_simInit) {
        resetSimulation();
    }
}

void TuringMachine::on_inc_speed_clicked() {
    int newInterval = m_timerInterval - 100;
    if (newInterval < 20) newInterval = 20;
    if (newInterval != m_timerInterval) {
        m_timerInterval = newInterval;
        restartTimer();
        UpdateSpeedDisplay();
    }
}

void TuringMachine::on_dec_speed_clicked() {
    int newInterval = m_timerInterval + 100;
    if (newInterval > 2000) newInterval = 2000;
    if (newInterval != m_timerInterval) {
        m_timerInterval = newInterval;
        restartTimer();
        UpdateSpeedDisplay();
    }
}

void TuringMachine::executeStep() {
    if (!m_simInit || m_tapeSymbols.isEmpty() || m_curState.isEmpty()) return;

    QString curSymbol = m_tapeSymbols[m_index];
    QString key = m_curState + ":" + curSymbol;
    const TuringRules& rule = m_transitions[key];

    if (!rule.newSymbol.isEmpty()) {
        m_tapeSymbols[m_index] = rule.newSymbol;
    }

    if (rule.direction == "L") {
        moveLeft();
    } else if (rule.direction == "R") {
        moveRight();
    } else {
        UpdateView();
    }

    if (!rule.newState.isEmpty()) {
        m_curState = rule.newState;
    }

    updateStateDisplay();

    if (rule.halt) {
        stopTimer();
        QMessageBox::information(this, "Остановка", "Выполнена команда '!'");
        m_simInit = false;
        ui->pause->setEnabled(false);
        ui->stop->setEnabled(true);
        ui->play->setEnabled(true);
        ui->step->setEnabled(true);
        return;
    }
}

void TuringMachine::StartSimulation() {
    QStringList states = collectStateNames();
    m_initState = states.first();
    m_curState = m_initState;

    m_index = m_initIndex;
    UpdateView();
    updateStateDisplay();

    m_simInit = true; 
    m_timer->setInterval(m_timerInterval);
    ui->table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->play->setEnabled(true);
    ui->step->setEnabled(true);
    ui->pause->setEnabled(false);
    ui->stop->setEnabled(true);

    m_timerActive = false;
}

void TuringMachine::stopTimer() {
    if (m_timer ->isActive()) {
        m_timer->stop();
    }
    m_timerActive = false;
    ui->play->setEnabled(true);
    ui->pause->setEnabled(false);
}

void TuringMachine::resetSimulation() {
    if (!m_initInput.isEmpty()) {
        CreateTape(m_initInput);
    }
    m_curState = m_initState;
    updateStateDisplay();
    m_simInit = false;

    ui->table->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui->pause->setEnabled(false);
    ui->stop->setEnabled(false);
}

void TuringMachine::updateStateDisplay() {
    QTableWidget* tw = ui->table;
    QStringList stateNames = collectStateNames();
    int curRow = stateNames.indexOf(m_curState);

    for (int row = 0; row < tw->rowCount(); ++row) {
        bool isActive = (row == curRow);
        for (int col = 0; col < tw->columnCount(); ++col) {
            QTableWidgetItem* item = tw->item(row, col);
            if (item) {
                if (isActive) {
                    item->setBackground(QBrush(QColor(0xE0, 0xF0, 0xFF)));
                } else {
                    item->setBackground(QBrush(QColor(Qt::white)));
                }
            }
        }
    }
}

void TuringMachine::moveLeft() {
    if (m_index == 0) {
        m_tapeSymbols.prepend("λ");
        ++m_index;
        ++m_viewOffset;
    }
    --m_index;

    int border = m_viewOffset + VISIBLE_COLS / 3;
    if (m_index < border) {
        m_viewOffset = qMax(0, m_viewOffset - VISIBLE_COLS / 3);
    }

    UpdateView();
    UpdateHead();
}

void TuringMachine::moveRight() {
    if (m_index == m_tapeSymbols.size() - 1) {
        m_tapeSymbols.append("λ");
    }
    ++m_index;

    int border = m_viewOffset + VISIBLE_COLS - 1 - VISIBLE_COLS / 3;
    if (m_index > border) {
        m_viewOffset = qMin(m_tapeSymbols.size() - VISIBLE_COLS, m_viewOffset + VISIBLE_COLS / 3);
    }

    UpdateView();
    UpdateHead();
}

QStringList TuringMachine::collectStateNames() const {
    QStringList names;
    QTableWidget* tw = ui->table;
    for (int row = 0; row < tw->rowCount(); ++row) {
        QTableWidgetItem* item = tw->item(row, 0);
        if (item) {
            names << item->text().trimmed();
        }
    }
    return names;
}

QString TuringMachine::validateAndLoadRules() {
    QTableWidget* tw = ui->table;
    bool has_halt = false;

    QStringList stateNames = collectStateNames();
    m_transitions.clear();
    QStringList errors;

    for(int row = 0; row < tw->rowCount(); ++row) {
        QString curState = tw->item(row, 0)->text().trimmed();
        for (int col = 1; col < tw->columnCount(); ++col) {
            QString symbol = m_allSymbols[col - 1];
            QTableWidgetItem* cell = tw->item(row, col);
            QString text = cell ? cell->text().trimmed() : QString();

            if (text.isEmpty()) continue;

            bool ok = false;


            QString error;
            TuringRules rule = parseRules(text, stateNames, ok, error);
            if (!ok) {
                resetSimulation();
                errors << QString("Строка %1, столбец '%2': %3").arg(row + 1).arg(symbol).arg(error);
                continue;
            }
            if (ok && rule.valid && rule.halt) {
                has_halt= true;
            }

            QString key = curState + ":" + symbol;
            m_transitions[key] = rule;
        }
    }

    if (!errors.isEmpty()) {
        resetSimulation();
        return errors.join("\n");
    }

    if (!has_halt) {
        resetSimulation();
        return "В таблице должна быть команда '!'(остановка)";
    }

    return QString();
}

TuringRules TuringMachine::parseRules(const QString &text,
                                      const QStringList& stateNames, bool &ok, QString &error) const {
    ok = true;
    error.clear();
    TuringRules rule;
    rule.valid = false;

    QStringList parts;
    for (const QString& p : text.split(',', Qt::KeepEmptyParts)) {
        QString trim = p.trimmed();
        if (!trim.isEmpty())
            parts.append(trim);
    }

    bool gotSymbol = false;
    bool gotDirection = false;
    bool gotState = false;
    bool halt = false;

    for (const QString& part : parts) {
        if (part == "!") {
            halt = true;
            continue;
        }

        if (m_allSymbols.contains(part)) {
            if (gotSymbol) {
                ok = false;
                error = "Два символа алфавита (ожидался порядок: символ -> направление -> состояние)";
                return rule;
            }
            if (gotDirection || gotState) {
                ok = false;
                error = "Символ не может следовать после направления или состояния";
                return rule;
            }
            rule.newSymbol = part;
            gotSymbol = true;
        }
        else if (part == "L" || part == "R") {
            if (gotDirection) {
                ok = false;
                error = "Два направления";
                return rule;
            }
            if (gotState) {
                ok = false;
                error = "Направление не может следовать после состояния";
                return rule;
            }
            rule.direction = part;
            gotDirection = true;
        }
        else if (stateNames.contains(part)) {
            if (gotState) {
                ok = false;
                error = "Два состояния";
                return rule;
            }
            gotState = true;
            rule.newState = part;
        }
        else {
            ok = false;
            error = QString("Неизвестный компонент '%1'").arg(part);
            return rule;
        }
    }

    rule.halt = halt;
    if (!rule.newSymbol.isEmpty() || !rule.direction.isEmpty() || !rule.newState.isEmpty() || halt) {
        rule.valid = true;
    }

    return rule;
}

void TuringMachine::restartTimer() {
    if (m_timerActive) {
        m_timer->stop();
        m_timer->setInterval(m_timerInterval);
        m_timer->start();
    } else {
        m_timer->setInterval(m_timerInterval);
    }
}

void TuringMachine::UpdateSpeedDisplay() {
    QString tooltip = QString("Интервал: %1 мс").arg(m_timerInterval);
    ui->inc_speed->setToolTip(tooltip);
    ui->dec_speed->setToolTip(tooltip);
}

void TuringMachine::on_change_alph_clicked() {
    emit alph_change(m_alphabet, m_addAlphabet);
}

void TuringMachine::applyAlphabetUpdate(const QString& mainAlph, const QString& addAlph) {
    QStringList newMain, newAdd;
    for (QChar ch : mainAlph) {
        if (!ch.isSpace()) newMain << ch;
    }
    for (QChar ch : addAlph) {
        if (!ch.isSpace()) newAdd << ch;
    }

    QSet<QString> setMain(newMain.begin(), newMain.end());
    if (setMain.size() != newMain.size()) {
        QMessageBox::warning(this, "Ошибка", "Основной алфавит содержит повторяющиеся символы.");
        return;
    }
    QSet<QString> setAdd(newAdd.begin(), newAdd.end());
    if (setAdd.size() != newAdd.size()) {
        QMessageBox::warning(this, "Ошибка", "Дополнительный алфавит содержит повторяющиеся символы.");
        return;
    }
    if (setMain.intersects(setAdd)) {
        QMessageBox::warning(this, "Ошибка", "Основной и дополнительный алфавиты не должны пересекаться.");
        return;
    }

    QMap<QString, QString> oldRules;
    QTableWidget* tw = ui->table;
    QStringList oldStates = collectStateNames();
    for (int row = 0; row < tw->rowCount(); ++row) {
        QString state = tw->item(row, 0)->text().trimmed();
        for (int col = 1; col < tw->columnCount(); ++col) {
            QString symbol = m_allSymbols.value(col - 1);
            QTableWidgetItem* item = tw->item(row, col);
            if (item && !item->text().isEmpty()) {
                oldRules[state + ":" + symbol] = item->text();
            }
        }
    }

    m_alphabet = mainAlph;
    m_addAlphabet = addAlph;

    CreateTable();

    tw = ui->table;
    tw->removeRow(0);
    for (const QString& stateName : oldStates) {
        int newRow = tw->rowCount();
        tw->insertRow(newRow);
        tw->setItem(newRow, 0, new QTableWidgetItem(stateName));
    }
    m_nextState = oldStates.size() - 1;

    for (auto it = oldRules.begin(); it != oldRules.end(); ++it) {
        QStringList keyParts = it.key().split(':');
        if (keyParts.size() != 2) continue;
        QString state = keyParts[0];
        QString symbol = keyParts[1];
        int row = oldStates.indexOf(state);
        int col = m_allSymbols.indexOf(symbol);
        if (row >= 0 && col >= 0) {
            tw->setItem(row, col + 1, new QTableWidgetItem(it.value()));
        }
    }

    m_allowed.clear();
    for (QChar ch : m_alphabet) {
        if (!ch.isSpace()) m_allowed.insert(ch);
    }
    for (QChar ch : m_addAlphabet) {
        if (!ch.isSpace()) m_allowed.insert(ch);
    }

    ui->tape->clear();
    m_tapeSymbols.clear();
    if (m_timerActive) stopTimer();
    m_simInit = false;
}
