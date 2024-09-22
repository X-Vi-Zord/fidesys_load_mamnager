#ifndef LOAD_FORM_VECTOR_H
#define LOAD_FORM_VECTOR_H

#include <QWidget>
#include <QJsonObject>
#include <QLineEdit>


namespace Ui {
class load_form_vector;
}

class load_form_vector : public QWidget
{
    Q_OBJECT

public:
    explicit load_form_vector(QWidget *parent = nullptr);
    ~load_form_vector();

public slots:

    void set_form(QJsonObject*);

signals:

    void new_load(QJsonObject*,int const);

private slots:
    void on_pushButton_clicked();

private:
    Ui::load_form_vector *ui;
    QJsonObject *object;
    std::array <QLineEdit*, 6> lanes;

};

#endif // LOAD_FORM_VECTOR_H
