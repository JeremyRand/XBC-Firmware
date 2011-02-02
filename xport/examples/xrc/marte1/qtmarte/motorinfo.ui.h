/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

void MotorInfo::init()
{
    m_motorNum = -1;
}

void MotorInfo::destroy()
{
}

int MotorInfo::getMotorVelocity()
{
 return MotorVelocity->value();
}

int MotorInfo::getMotorPosition()
{
 return MotorPosition->value();
}

void MotorInfo::setMotorLabel(int motorNum, const QString& motorTxt)
{
    m_motorNum = motorNum;
    MotorLabel->setText(motorTxt);
}

void MotorInfo::setMotorVelocity(int vel)
{
    MotorCurVelocity->setText(QString::number(vel));
}

void MotorInfo::setMotorCurrentPosition(int pos)
{
    MotorCurPosition->setText(QString::number(pos));
}

void MotorInfo::setMotorGoalPosition(int pos)
{
    MotorGoalPosition->setText(QString::number(pos));
}

void MotorInfo::emitHold()
{
    emit HoldMotorCommand(m_motorNum);
}

void MotorInfo::emitGo()
{
    emit RunMotorCommand(m_motorNum);
}

void MotorInfo::emitSetPosition()
{
    emit SetPositionCommand(m_motorNum);
}

void MotorInfo::enableGo()
{
    MotorGoButton->setEnabled(true);
    MotorHoldButton->setEnabled(true);
    SetPosButton->setEnabled(true);
}

void MotorInfo::disableGo()
{
    MotorGoButton->setEnabled(false);
    MotorHoldButton->setEnabled(false);
    SetPosButton->setEnabled(false);
}

