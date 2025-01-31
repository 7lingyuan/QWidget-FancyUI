#include "sidebar.h"

Sidebar::Sidebar(QWidget *parent,int initialWidth)
    : QWidget{parent},
    animation{new QPropertyAnimation(this,"size")},
    verticalLayout{new QVBoxLayout(this)}
{
    this->verticalLayout->setSpacing(4);
    this->verticalLayout->setContentsMargins(5, 5, 5, 5);

    this->animation->setDuration(150);

    connect(this->animation,&QPropertyAnimation::finished,this,[&]{
        if(this->initialSize.height()!=this->window()->height())
        {
            //动画结束,高度和父窗口不等,说明动画进行的过程中改变了窗口大小,需重新设置高度
            this->initialSize.rheight()=window()->height();
            this->endSize.rheight() = this->initialSize.rheight();
            this->animation->setStartValue(this->initialSize);
            this->animation->setEndValue(this->endSize);
            this->resize(width(),window()->height());
        }
        //设置展开状态和动画运行状态,根据展开状态发射相应信号
        this->isExpanded = !this->isExpanded;
        this->isAnimationRunning = false;
        if(this->isExpanded)
            emit this->expandFinished();
        else
            emit this->shrinkFinished();
        emit this->finished();
    });

    //动画起始高度和父窗口高度一样,宽度为初始宽度,之后不再更改宽度
    this->initialSize.rwidth() = initialWidth;
    this->initialSize.rheight() = this->window()->height();
    //动画结束高度和动画起始高度始终相等,动画结束宽度为动画起始宽度加上展开增加宽度
    this->endSize.rwidth() = this->initialSize.rwidth()+increasedWidth;
    this->endSize.rheight() = this->initialSize.rheight();

    this->resize(initialWidth, window()->height());
}

Sidebar::~Sidebar()
{
    animation->deleteLater();
}

void Sidebar::autoExpand()
{
    this->expand();
    this->shrink();
}

void Sidebar::expand()
{
    //当前不是展开状态
    if(!this->isExpanded && !this->isAnimationRunning)
    {
        this->animation->setDirection(QAbstractAnimation::Direction::Forward);//正向
        this->animation->start();
        this->isAnimationRunning = true;
        emit this->expandStart();
    }
}

void Sidebar::shrink()
{
    if(this->isExpanded && !this->isAnimationRunning)
    {
        this->animation->setDirection(QAbstractAnimation::Direction::Backward);//逆向
        this->animation->start();
        this->isAnimationRunning = true;
        emit this->shrinkStart();
    }
}

void Sidebar::addItem(QLayoutItem * item)
{
    this->verticalLayout->addItem(item);
}

void Sidebar::setIncreasedWidth(quint32 increasedWidth)
{
    this->increasedWidth = increasedWidth;
    this->endSize.rwidth() = this->initialSize.rwidth()+increasedWidth;
    this->animation->setEndValue(this->endSize);
}

void Sidebar::setExpandTime(int ms)
{
    this->animation->setDuration(ms);
}

void Sidebar::setBackgroundBrush(const QBrush &brush)
{
    this->backgroundBrush = brush;
}

void Sidebar::setBorderLinePen(const QPen &pen)
{
    this->borderLinePen = pen;
}

int Sidebar::ChildsCumulativeHeight()
{
    int cumulativeHeight = 0;//累计高度

    //查找直接子控件
    auto childs = this->findChildren<QWidget*>(Qt::FindChildOption::FindDirectChildrenOnly);

    //累加直接子控件高度
    for(auto& child : childs)cumulativeHeight += child->width();

    //累加布局边距和控件间距
    cumulativeHeight += layout()->contentsMargins().top();
    cumulativeHeight += layout()->contentsMargins().bottom();
    cumulativeHeight += layout()->spacing()*(childs.count()-1);

    return cumulativeHeight;
}

void Sidebar::addWidget(QWidget *btn)
{
    this->verticalLayout->addWidget(btn);
}

void Sidebar::resizeEvent(QResizeEvent *)
{
    //动画未运行时改变,父窗口高度改变,需重设动画起始和结束高度
    //这里不判断高度是否相等(性能差不多?? 但是需要增加额外的初始化代码)
    if(!this->isAnimationRunning)//&& this->height()!=this->window()->height()
    {
        this->initialSize.rheight() = this->height();
        this->endSize.rheight() = this->initialSize.rheight();
        this->animation->setStartValue(this->initialSize);
        this->animation->setEndValue(this->endSize);
    }
}

void Sidebar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setBrush(this->backgroundBrush);
    painter.setPen(Qt::PenStyle::NoPen);
    painter.drawRect(this->rect());
    painter.setPen(this->borderLinePen);
    painter.drawLine(width(),0,width(),height());
}
