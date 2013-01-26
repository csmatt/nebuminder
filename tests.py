#!/usr/bin/python
AM = 0
PM = 1
START_HOUR = 11
START_PM = AM
ALARM_INTERVAL=1
LATEST_ALARM_SET_HOUR=7 #11-ALARM_INTERVAL
LATEST_ALARM_SET_PM=PM

def setNextAlarm(crntHr, crntPM):
    resetToStartHour = False
    nextAlarmPM = crntPM

    # This block sets resetToStartHour just to make things a little easier to understand (as opposed to having one big if-statement)
    if crntHr == 12 or crntHr == 0: # special case to handle 12am and 12pm
        if crntHr == 0: # if we're at 12am, reset to start hour (otherwise we're at 12pm and don't need to reset)
            resetToStartHour = True
    elif (crntPM == PM and crntHr > LATEST_ALARM_SET_HOUR) or (crntPM == AM and crntHr < START_HOUR):
        resetToStartHour = True

    if resetToStartHour:
        nextAlarmHr = START_HOUR
        nextAlarmPM = START_PM
    else:
        # we'll deal with waking hour alarm setting here
        nextAlarmHr = crntHr + ALARM_INTERVAL
        if nextAlarmHr >= 12: # since nextAlarmHr was calculated by merely adding ALARM_INTERVAL to crntHr, we know that anything >= 12 should be set to PM
            nextAlarmPM = PM
            if not nextAlarmHr == 12: # if we're > 12, subtract 12 to get the 12hr version of the nextAlarmHr
                nextAlarmHr -= 12

    return [nextAlarmHr, nextAlarmPM, resetToStartHour]

def testWrapper(crntHr, crntPM, alarmHr, alarmPM, testInfo):
    result = setNextAlarm(crntHr,crntPM)
    testResult = result[0] == alarmHr and result[1] == alarmPM
    resetToStartHour = result[2]
    if not testResult:
        print '%s\t%d\t%d\t%d/%d\t%d/%d\t%s\t%s\n' % (testResult, crntHr, crntPM, alarmHr, result[0], alarmPM, result[1], resetToStartHour, testInfo)
    return testResult

def oneHourTests():
    testWrapper(0,AM,11,AM, '12am to 11am')
    testWrapper(1,AM,11,AM, '1am to 11am')
    testWrapper(2,AM,11,AM, '2am to 11am')
    testWrapper(3,AM,11,AM, '3am to 11am')
    testWrapper(4,AM,11,AM, '4am to 11am')
    testWrapper(5,AM,11,AM, '5am to 11am')
    testWrapper(6,AM,11,AM, '6am to 11am')
    testWrapper(7,AM,11,AM, '7am to 11am')
    testWrapper(8,AM,11,AM, '8am to 11am')
    testWrapper(9,AM,11,AM, '9am to 11am')
    testWrapper(10,AM,11,AM, '10am to 11pm')
    testWrapper(11,AM,12,PM, '11am to 12pm')

    testWrapper(12,PM,1,PM, '12pm to 1pm')
    testWrapper(1,PM,2,PM, '1pm to 2pm')
    testWrapper(2,PM,3,PM, '2pm to 3pm')
    testWrapper(3,PM,4,PM, '3pm to 4pm')
    testWrapper(4,PM,5,PM, '4pm to 5pm')
    testWrapper(5,PM,6,PM, '5pm to 6pm')
    testWrapper(6,PM,7,PM, '6pm to 7pm')
    testWrapper(7,PM,8,PM, '7pm to 8pm')
    testWrapper(8,PM,11,AM, '8pm to 11am')
    testWrapper(9,PM,11,AM, '9pm to 11am')
    testWrapper(10,PM,11,AM, '10pm to 11am')
    testWrapper(11,PM,11,AM, '11pm to 11am')

def twoHourTests():
    testWrapper(0,AM,11,AM, '12am to 11am')
    testWrapper(1,AM,11,AM, '1am to 11am')
    testWrapper(2,AM,11,AM, '2am to 11am')
    testWrapper(3,AM,11,AM, '3am to 11am')
    testWrapper(4,AM,11,AM, '4am to 11am')
    testWrapper(5,AM,11,AM, '5am to 11am')
    testWrapper(6,AM,11,AM, '6am to 11am')
    testWrapper(7,AM,11,AM, '7am to 11am')
    testWrapper(8,AM,11,AM, '8am to 11am')
    testWrapper(9,AM,11,AM, '9am to 11am')
    testWrapper(10,AM,11,AM, '10am to 11am')
    testWrapper(11,AM,1,PM, '11am to 1pm')

    testWrapper(12,PM,2,PM, '12pm to 2pm')
    testWrapper(1,PM,3,PM, '1pm to 3pm')
    testWrapper(2,PM,4,PM, '2pm to 4pm')
    testWrapper(3,PM,5,PM, '3pm to 5pm')
    testWrapper(4,PM,6,PM, '4pm to 6pm')
    testWrapper(5,PM,7,PM, '5pm to 7pm')
    testWrapper(6,PM,8,PM, '6pm to 8pm')
    testWrapper(7,PM,9,PM, '7pm to 9pm')
    testWrapper(8,PM,11,AM, '8pm to 11am')
    testWrapper(9,PM,11,AM, '9pm to 11am')
    testWrapper(10,PM,11,AM, '10pm to 11am')
    testWrapper(11,PM,11,AM, '11pm to 11am')

def threeHourTests():
    testWrapper(0,AM,11,AM, '12am to 11am')
    testWrapper(1,AM,11,AM, '1am to 11am')
    testWrapper(2,AM,11,AM, '2am to 11am')
    testWrapper(3,AM,11,AM, '3am to 11am')
    testWrapper(4,AM,11,AM, '4am to 11am')
    testWrapper(5,AM,11,AM, '5am to 11am')
    testWrapper(6,AM,11,AM, '6am to 11am')
    testWrapper(7,AM,11,AM, '7am to 11am')
    testWrapper(8,AM,11,AM, '8am to 11am')
    testWrapper(9,AM,11,AM, '9am to 11am')
    testWrapper(10,AM,11,AM, '10am to 11am')
    testWrapper(11,AM,2,PM, '11am to 2pm')

    testWrapper(12,PM,3,PM, '12pm to 3pm')
    testWrapper(1,PM,4,PM, '1pm to 4pm')
    testWrapper(2,PM,5,PM, '2pm to 5pm')
    testWrapper(3,PM,6,PM, '3pm to 6pm')
    testWrapper(4,PM,7,PM, '4pm to 7pm')
    testWrapper(5,PM,8,PM, '5pm to 8pm')
    testWrapper(6,PM,9,PM, '6pm to 9pm')
    testWrapper(7,PM,10,PM, '7pm to 10pm')
    testWrapper(8,PM,11,AM, '8pm to 11am')
    testWrapper(9,PM,11,AM, '9pm to 11am')
    testWrapper(10,PM,11,AM, '10pm to 11am')
    testWrapper(11,PM,11,AM, '11pm to 11am')

def fourHourTests():
    testWrapper(0,AM,11,AM, '12am to 11am')
    testWrapper(1,AM,11,AM, '1am to 11am')
    testWrapper(2,AM,11,AM, '2am to 11am')
    testWrapper(3,AM,11,AM, '3am to 11am')
    testWrapper(4,AM,11,AM, '4am to 11am')
    testWrapper(5,AM,11,AM, '5am to 11am')
    testWrapper(6,AM,11,AM, '6am to 11am')
    testWrapper(7,AM,11,AM, '7am to 11am')
    testWrapper(8,AM,11,AM, '8am to 11am')
    testWrapper(9,AM,11,AM, '9am to 11am')
    testWrapper(10,AM,11,AM, '10am to 11am')
    testWrapper(11,AM,3,PM, '11am to 3pm')

    testWrapper(12,PM,4,PM, '12pm to 4pm')
    testWrapper(1,PM,5,PM, '1pm to 5pm')
    testWrapper(2,PM,6,PM, '2pm to 6pm')
    testWrapper(3,PM,7,PM, '3pm to 7pm')
    testWrapper(4,PM,8,PM, '4pm to 8pm')
    testWrapper(5,PM,9,PM, '5pm to 9pm')
    testWrapper(6,PM,10,PM, '6pm to 10pm')
    testWrapper(7,PM,11,PM, '7pm to 11pm')
    testWrapper(8,PM,11,AM, '8pm to 11am')
    testWrapper(9,PM,11,AM, '9pm to 11am')
    testWrapper(10,PM,11,AM, '10pm to 11am')
    testWrapper(11,PM,11,AM, '11pm to 11am')

def runTests():
    print "== Running Tests =="
    oneHourTests()

if __name__ == '__main__':
    runTests()
