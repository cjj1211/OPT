#include "CBinding.h"

CBinding::CBinding(QWidget* parent)
    :QWidget(parent)
{
    ui = std::make_unique<Ui::Binding>();
    ui->setupUi(this);
}

CBinding::~CBinding()
{
}
