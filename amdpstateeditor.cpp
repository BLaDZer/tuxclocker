#include "amdpstateeditor.h"
#include "ui_amdpstateeditor.h"

amdPstateEditor::amdPstateEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::amdPstateEditor)
{
    ui->setupUi(this);
}

amdPstateEditor::~amdPstateEditor()
{
    delete ui;
}
void amdPstateEditor::generateUI(gputypes *newtypes, int GPUIndex)
{
    types = newtypes;
    gpuidx = GPUIndex;
    qDebug() << newtypes->gpuCount << "gpus in pointer";
    QWidget *lower = new QWidget;
    QWidget *upper = new QWidget;
    QHBoxLayout *ulo = new QHBoxLayout;
    QHBoxLayout *llo = new QHBoxLayout;
    for (int i=0; i<newtypes->GPUList[gpuidx].coreclocks.size(); i++) {
        corePstate state;
        QGridLayout *glo = new QGridLayout;
        QLabel *voltlabel = new QLabel;
        QLabel *freqlabel = new QLabel;
        QLabel *pstatelabel = new QLabel;
        voltlabel->setText("mV");
        freqlabel->setText("MHz");
        pstatelabel->setText("Core pstate "+QString::number(i));
        QSlider *freqslider = new QSlider;
        QSlider *voltslider = new QSlider;
        QSpinBox *freqspinbox = new QSpinBox;
        QSpinBox *voltspinbox = new QSpinBox;
        connect(freqslider, SIGNAL(valueChanged(int)), freqspinbox, SLOT(setValue(int)));
        connect(freqspinbox, SIGNAL(valueChanged(int)), freqslider, SLOT(setValue(int)));
        connect(voltspinbox, SIGNAL(valueChanged(int)), voltslider, SLOT(setValue(int)));
        connect(voltslider, SIGNAL(valueChanged(int)), voltspinbox, SLOT(setValue(int)));

        freqslider->setRange(newtypes->GPUList[gpuidx].minCoreClkLimit, newtypes->GPUList[gpuidx].maxCoreClkLimit);
        freqspinbox->setRange(newtypes->GPUList[gpuidx].minCoreClkLimit, newtypes->GPUList[gpuidx].maxCoreClkLimit);

        voltslider->setRange(newtypes->GPUList[gpuidx].minVoltageLimit, newtypes->GPUList[gpuidx].maxVoltageLimit);
        voltspinbox->setRange(newtypes->GPUList[gpuidx].minVoltageLimit, newtypes->GPUList[gpuidx].maxVoltageLimit);

        voltspinbox->setValue(newtypes->GPUList[gpuidx].corevolts[i]);
        freqspinbox->setValue(newtypes->GPUList[gpuidx].coreclocks[i]);

        glo->addWidget(pstatelabel);
        glo->addWidget(freqlabel, 1, 0);
        glo->addWidget(voltlabel, 1, 1);
        glo->addWidget(freqslider, 2, 0);
        glo->addWidget(voltslider, 2, 1);
        glo->addWidget(freqspinbox, 3, 0);
        glo->addWidget(voltspinbox, 3, 1);
        QWidget *freqsliderowdg = new QWidget;
        freqsliderowdg->setLayout(glo);
        ulo->addWidget(freqsliderowdg);

        state.voltspinbox = voltspinbox;
        state.freqspinbox = freqspinbox;
        corePstates.append(state);
    }

    for (int i=0; i<newtypes->GPUList[gpuidx].memclocks.size(); i++) {
        memPstate state;
        QGridLayout *glo = new QGridLayout;
        QLabel *voltlabel = new QLabel;
        QLabel *freqlabel = new QLabel;
        QLabel *pstatelabel = new QLabel;
        voltlabel->setText("mV");
        freqlabel->setText("MHz");
        pstatelabel->setText("Memory pstate "+QString::number(i));
        QSlider *freqslider = new QSlider;
        QSlider *voltslider = new QSlider;
        QSpinBox *freqspinbox = new QSpinBox;
        QSpinBox *voltspinbox = new QSpinBox;
        connect(freqslider, SIGNAL(valueChanged(int)), freqspinbox, SLOT(setValue(int)));
        connect(freqspinbox, SIGNAL(valueChanged(int)), freqslider, SLOT(setValue(int)));
        connect(voltspinbox, SIGNAL(valueChanged(int)), voltslider, SLOT(setValue(int)));
        connect(voltslider, SIGNAL(valueChanged(int)), voltspinbox, SLOT(setValue(int)));

        freqslider->setRange(newtypes->GPUList[gpuidx].minMemClkLimit, newtypes->GPUList[gpuidx].maxMemClkLimit);
        freqspinbox->setRange(newtypes->GPUList[gpuidx].minMemClkLimit, newtypes->GPUList[gpuidx].maxMemClkLimit);

        voltslider->setRange(newtypes->GPUList[gpuidx].minMemClkLimit, newtypes->GPUList[gpuidx].maxMemClkLimit);
        voltspinbox->setRange(newtypes->GPUList[gpuidx].minMemClkLimit, newtypes->GPUList[gpuidx].maxMemClkLimit);

        voltspinbox->setValue(newtypes->GPUList[gpuidx].memvolts[i]);
        freqspinbox->setValue(newtypes->GPUList[gpuidx].memclocks[i]);

        glo->addWidget(pstatelabel);
        glo->addWidget(freqlabel, 1, 0);
        glo->addWidget(voltlabel, 1, 1);
        glo->addWidget(freqslider, 2, 0);
        glo->addWidget(voltslider, 2, 1);
        glo->addWidget(freqspinbox, 3, 0);
        glo->addWidget(voltspinbox, 3, 1);
        QWidget *freqsliderowdg = new QWidget;
        freqsliderowdg->setLayout(glo);
        llo->addWidget(freqsliderowdg);
        state.voltspinbox = voltspinbox;
        state.freqspinbox = freqspinbox;
        memPstates.append(state);
    }
    // Add an apply button
    QPushButton *applyButton = new QPushButton;
    connect(applyButton, SIGNAL(clicked()), SLOT(applyValues()));
    applyButton->setText("Apply values");
    llo->addWidget(applyButton);
    // Add a reset button
    QPushButton *resetButton = new QPushButton;
    connect(resetButton, SIGNAL(clicked()), SLOT(resetPstates()));
    resetButton->setText("Reset to defaults");
    llo->addWidget(resetButton);

    lower->setLayout(llo);
    upper->setLayout(ulo);

    QVBoxLayout *mainlo = new QVBoxLayout;
    mainlo->addWidget(upper);
    mainlo->addWidget(lower);
    ui->centralWidget->setLayout(mainlo);
}
bool amdPstateEditor::applyValues()
{
    qDebug("Applying values");
    QProcess proc;
    QString volt;
    QString freq;
    QString cmd = "pkexec /bin/sh -c \"";
    bool changedState = false;
    // Apply core pstates
    for (int i=0; i<corePstates.size(); i++) {
        if ((corePstates[i].freqspinbox->value() != types->GPUList[gpuidx].coreclocks[i]) || (corePstates[i].voltspinbox->value() != types->GPUList[gpuidx].corevolts[i])) {
            changedState = true;
            volt = QString::number(corePstates[i].voltspinbox->value());
            freq = QString::number(corePstates[i].freqspinbox->value());
            cmd.append("echo 's "+ QString::number(i) + " "+ freq +" "+ volt +"' "+"> /sys/class/drm/card"+QString::number(types->GPUList[gpuidx].fsindex)+"/device/pp_od_clk_voltage & ");
            qDebug() << cmd;
            proc.start(cmd);
            proc.waitForFinished();
        }
    }
    // Apply memory pstates
    for (int i=0; i<memPstates.size(); i++) {
        if ((memPstates[i].freqspinbox->value() != types->GPUList[gpuidx].memclocks[i]) || (memPstates[i].voltspinbox->value() != types->GPUList[gpuidx].memvolts[i])) {
            changedState = true;
            volt = QString::number(memPstates[i].voltspinbox->value());
            freq = QString::number(memPstates[i].freqspinbox->value());
            cmd.append("echo 'm "+ QString::number(i) + " "+ freq +" "+ volt +"' "+"> /sys/class/drm/card"+QString::number(types->GPUList[gpuidx].fsindex)+"/device/pp_od_clk_voltage & ");
            qDebug() << cmd;
            proc.start(cmd);
            proc.waitForFinished();
        }
    }
    if (changedState) {
        cmd.append("\"");
        proc.start(cmd);
        proc.waitForFinished(-1);
    }

    return true;
}
bool amdPstateEditor::resetPstates()
{
    bool ret = false;
    QProcess proc;
    proc.start("pkexec /bin/sh -c \"echo 'r' > /sys/class/drm/card" + QString::number(types->GPUList[gpuidx].fsindex)+"/device/pp_od_clk_voltage\"");
    proc.waitForFinished(-1);
    if (proc.exitCode() == 0) {
        ret = true;
    }
    return ret;
}
