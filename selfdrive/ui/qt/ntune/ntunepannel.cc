#include "ntunepannel.h"
#include "ntunewidget.h"
#include <QStringListModel>
#include <QList>
#include <QHBoxLayout>
#include <QFont>
#include <QDebug>

nTuneMainWidget::nTuneMainWidget(QWidget *parent)
    : QWidget{parent} {

    QList<QString> mainTitles = {
    "기본", "크루즈", "핸들"
    };

    QList<QList<TuneItemInfo>> mainItems = {
    {
        TuneItemInfo("common.json", "pathFactor", "차선추종 강도", tr("커브에서 차선을 따라가는 힘입니다. 바깥으로 밀리면 올리고, 안쪽으로 과하게 파고들면 내리세요."),
                     0.96f, 0.9f, 1.1f, 0.01f, 2),
        TuneItemInfo("common.json", "steerActuatorDelay", "핸들 반응 지연", tr("핸들 명령 후 실제 차량이 반응하기까지의 시간입니다. 커브 반응이 늦으면 조금 올리고, 잔조향이 많으면 내리세요."),
                     0.2f, 0.0f, 0.8f, 0.05f, 2),
    },
    {
        TuneItemInfo("scc_v2.json", "longStartingFactor", "출발 가속 강도", tr("정지 후 출발할 때 가속 힘입니다. 출발이 답답하면 올리고, 튀어나가는 느낌이면 내리세요."),
                     1.4f, 0.7f, 1.65f, 0.1f, 1),
        TuneItemInfo("scc_v2.json", "longLeadSensitivity", "앞차 반응 민감도", tr("앞차 감속이나 출발에 반응하는 민감도입니다. 앞차 반응이 늦으면 올리고, 너무 자주 감속하면 내리세요."),
                     0.9f, 0.5f, 1.3f, 0.1f, 1),
    },
    {
        TuneItemInfo("lat_torque_v4.json", "latAccelFactor", "코너 조향 강도", tr("커브에서 핸들을 돌리는 힘입니다. 바깥으로 밀리면 올리고, 너무 과하게 꺾이면 내리세요."), 2.5f, 0.5f, 4.5f, 0.1f, 2),
        TuneItemInfo("lat_torque_v4.json", "friction", "핸들 마찰 보정", tr("핸들이 처음 움직일 때의 둔한 느낌을 보정합니다. 초반 조향이 답답하면 올리고, 잔조향이 많으면 내리세요."), 0.1f, 0.0f, 0.2f, 0.01f, 3),
        TuneItemInfo("lat_torque_v4.json", "angle_deadzone_v2", "미세조향 무시값", tr("아주 작은 핸들 움직임을 무시하는 범위입니다. 직진 중 좌우로 흔들리면 올리고, 반응이 둔하면 내리세요."), 0.0f, 0.0f, 2.0f, 0.01f, 3),
    },
    };

    setStyleSheet(R"(
        * {
          color: white;
          font-size: 55px;
        }
        SettingsWindow {
          background-color: black;
        }
        QStackedWidget, ScrollView, QListView {
          background-color: #292929;
          border-radius: 0px;
        }
        QTabBar::tab { padding: 10px; border-radius: 10px; background-color: transparent; border-bottom: none;}
        QTabBar::tab:selected { background-color: #555555; border-bottom: none;}
        QTabBar::tab:!selected { background-color: transparent; border-bottom: none;}
        QComboBox {
            background-color: transparent;
            border: 2px solid #555555;
            border-radius: 5px;
            font-size: 50px;
        }
        QComboBox QAbstractItemView {
            background: transparent;
            selection-background-color: #555555;
            font-size: 50px;
        }
    )");

    QHBoxLayout *layout = new QHBoxLayout(this);

    QStringList data;
    foreach (auto title, mainTitles) {
        data.append(title);
    }

    QStringListModel *model = new QStringListModel(data);
    listView = new QListView();
    listView->setStyleSheet(R"(
        QListView::item {  padding-top: 30px; padding-bottom: 30px; padding-left: 15px; padding-right: 15px;
            border-radius: 10px; font-weight: bold;}
        QListView::item:selected { background-color: #555555; }
        QListView::item:!selected { background-color: #00000000; }
    )");
    listView->setModel(model);
    BoldItemDelegate *delegate = new BoldItemDelegate(listView);
    listView->setItemDelegate(delegate);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QItemSelectionModel *selectionModel = listView->selectionModel();
    selectionModel->select(model->index(0, 0), QItemSelectionModel::Select);

    stackedWidget = new QStackedWidget;
    stackedWidget->setStyleSheet("QWidget { background-color: #404040; border-radius: 10px; }");
    foreach (auto items, mainItems) {
        stackedWidget->addWidget(new nTunePannel(items));
    }

    layout->addWidget(listView, 1);
    layout->addWidget(stackedWidget, 4);

    this->setLayout(layout);

    QObject::connect(listView->selectionModel(), &QItemSelectionModel::currentChanged,
                     [&](const QModelIndex &current, const QModelIndex &previous) {
                         qDebug() << "Selected item:" << current.row();
                         this->stackedWidget->setCurrentIndex(current.row());
                     });
}

nTunePannel::nTunePannel(QList<TuneItemInfo>& items, QWidget *parent)
    : QWidget{parent} {

    QVBoxLayout *layout = new QVBoxLayout(this);
    tabBar = new QTabBar;
    tabBar->setStyleSheet(R"(
        QTabBar::tab { font-weight: bold; font-size: 45px; }
    )");
    stackedWidget = new QStackedWidget;
    stackedWidget->setStyleSheet(R"(
        padding: 20px;
    )");

    foreach (const TuneItemInfo item, items) {
        tabBar->addTab(item.title);
        stackedWidget->addWidget(new nTuneWidget(item));
    }

    layout->addWidget(tabBar);
    layout->addWidget(stackedWidget);

    connect(tabBar, &QTabBar::currentChanged, stackedWidget, &QStackedWidget::setCurrentIndex);

    setMouseTracking(true);
}
