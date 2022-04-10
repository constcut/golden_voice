import QtQuick 2.7
import QtQuick.Controls 1.4 as QMLOld
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.0
import QtQuick.Window 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Controls.Styles 1.4



Item {

    y: 2 * parent.height / 4
    x: parent.width / 40
    height: parent.height / 2

    id: yearCalendarItem

    signal dateWasChosen

    property string currentDate: "today"

    SystemPalette {
        id: systemPalette
    }

    MouseArea
    {
        anchors.fill: parent
        onWheel: {
            if (wheel.angleDelta.y < 0) {
                if (scrollViewAnimation.running) {
                    scrollViewAnimation.stop()
                    scrollViewAnimation.to--
                    scrollViewAnimation.start()
                } else {
                    scrollViewAnimation.to = Math.round(view.offset - 1)
                    scrollViewAnimation.start()
                }
            } else if (wheel.angleDelta.y > 0) {
                if (scrollViewAnimation.running) {
                    scrollViewAnimation.stop()
                    scrollViewAnimation.to++
                    scrollViewAnimation.start()
                } else {
                    scrollViewAnimation.to = Math.round(view.offset + 1)
                    scrollViewAnimation.start()
                }
            }
        }
    }

    PathView
    {
        id: view
        property int itemAngle: 40.0
        property int itemSize: 300

        property string globalDate: "Сегодня"
        property date lastSelectedDate: new Date()

        property int lastChosenIndex: -1

        function setAllDatesTogether(dayValue, monthValue, yearValue, andIndex) {
            if (lastChosenIndex != -1) {

            }

            var indexDiff = currentIndex - andIndex

            if (indexDiff >= 5)
                indexDiff -= 12
            else if (indexDiff <= -5)
                indexDiff += 12

            if (indexDiff > 0) {
                while (indexDiff != 0) {
                    view.decrementCurrentIndex()
                    --indexDiff
                }
            } else if (indexDiff < 0) {
                while (indexDiff != 0) {
                    view.incrementCurrentIndex()
                    ++indexDiff
                }
            }
        }

        Component.onCompleted: {
            currentIndex = lastSelectedDate.getMonth()
        }

        anchors.fill: parent
        pathItemCount: 10
        preferredHighlightBegin: 0.5
        preferredHighlightEnd: 0.5
        interactive: true
        model: 12

        delegate: viewDelegate

        onDragStarted: {
            scrollViewAnimation.stop()
        }

        NumberAnimation on offset {
            id: scrollViewAnimation
            duration: 250
            easing.type: Easing.InOutQuad
        }

        path: Path {

            startX: 0
            startY: 0
            PathPercent {
                value: 0.0
            }
            PathAttribute {
                name: "z"
                value: 0
            }
            PathAttribute {
                name: "angle"
                value: view.itemAngle
            }
            PathAttribute {
                name: "origin"
                value: 0
            }

            PathLine {
                x: view.width * 0.4
                y: 0
            }
            PathPercent {
                value: 0.45
            }
            PathAttribute {
                name: "angle"
                value: view.itemAngle
            }
            PathAttribute {
                name: "origin"
                value: 0
            }
            PathAttribute {
                name: "z"
                value: 10
            }

            PathLine {
                relativeX: 0
                relativeY: 0
            }
            PathAttribute {
                name: "angle"
                value: 0.0
            }
            PathAttribute {
                name: "origin"
                value: 0
            }
            PathAttribute {
                name: "z"
                value: 10
            }

            PathLine {
                x: view.width * 0.6
                y: 0
            }
            PathPercent {
                value: 0.55
            }
            PathAttribute {
                name: "angle"
                value: 0.0
            }
            PathAttribute {
                name: "origin"
                value: 0
            }
            PathAttribute {
                name: "z"
                value: 10
            }

            PathLine {
                relativeX: 0
                relativeY: 0
            }
            PathAttribute {
                name: "angle"
                value: -view.itemAngle
            }
            PathAttribute {
                name: "origin"
                value: view.itemSize
            }
            PathAttribute {
                name: "z"
                value: 10
            }

            PathLine {
                x: view.width
                y: 0
            }
            PathPercent {
                value: 1
            }
            PathAttribute {
                name: "angle"
                value: -view.itemAngle
            }
            PathAttribute {
                name: "origin"
                value: view.itemSize
            }
            PathAttribute {
                name: "z"
                value: 0
            }
        }
    }

    Component
    {
        id: viewDelegate

        Rectangle {

            id: flipItem
            width: view.itemSize
            height: view.itemSize
            color: "white"
            z: PathView.z

            function updateByIndex(indexValue) {
                //console.log("Updating by index")
            }

            property var rotationAngle: PathView.angle
            property var rotationOrigin: PathView.origin

            transform: Rotation {
                id: rot //nice name haha
                axis {
                    x: 0
                    y: 1
                    z: 0
                }
                angle: rotationAngle
                origin.x: rotationOrigin
                origin.y: width
            }

            Rectangle
            {
                border.color: "#1563A5"
                border.width: 2
                color: "transparent"
                anchors.top: flipItem.top

                anchors.left: flipItem.left
                anchors.right: flipItem.right
                width: flipItem.width
                height: flipItem.height
                smooth: true
                antialiasing: true

                QMLOld.Calendar
                {
                    id: paintedCalendar

                    visibleMonth: index

                    width: 300 - 4
                    height: 300 - 4

                    x: 2
                    y: 2

                    function checkDateBirthEmpty(dateValue)
                    {
                        var isoDate = dateValue.getFullYear() + '-' + ('0' + (dateValue.getMonth() + 1)).slice(-2) + '-' + ('0' + dateValue.getDate()).slice(-2)
                        var dateString = isoDate.substring(8, 10) + "." + isoDate.substring(5, 7) + "." + isoDate.substring(0, 4)
                        var shortDate = isoDate.substring(8, 10) + "." + isoDate.substring(5, 7)

                        var noBirthList = [] //.getNoBirthdaysList()

                        for (var i = 0; i < noBirthList.length; ++i)
                        {
                            var thatDate = noBirthList[i]

                            if (thatDate === shortDate)
                                return true
                        }

                        return false
                    }

                    function checkDateIsForSet(dateValue)
                    {
                        var isoDate = dateValue.getFullYear() + '-' + ('0' + (dateValue.getMonth() + 1)).slice(-2) + '-' + ('0' + dateValue.getDate()).slice(-2)
                        var dateString = isoDate.substring(8, 10) + "." + isoDate.substring(5, 7) + "." + isoDate.substring(0, 4)
                        var shortDate = isoDate.substring(8, 10) + "." + isoDate.substring(5, 7)

                        var calendarMarks = [] // .getCalendarMarks();

                        for (var i = 0; i < calendarMarks.length; ++i)
                        {
                            var birthPartTop = calendarMarks[i].substring(0, 5)
                            if (shortDate === birthPartTop)
                                return true
                        }

                        return false
                    }

                    onClicked:
                    {
                        yearCalendarItem.currentDate = paintedCalendar.selectedDate
                        view.lastSelectedDate = paintedCalendar.selectedDate

                        var daySelected = yearCalendarItem.currentDate.substring(8, 10)
                        var monthSelected = yearCalendarItem.currentDate.substring(5, 7)
                        var yearSelected = yearCalendarItem.currentDate.substring(0, 4)

                        //can make back reset to index month right here
                        if (index !== parseInt(monthSelected, 10) - 1)
                            visibleMonth = index

                        view.setAllDatesTogether(daySelected, monthSelected,
                                                 yearSelected, index)

                        yearCalendarItem.currentDate = yearCalendarItem.currentDate.substring(8,10)
                                + "." + yearCalendarItem.currentDate.substring(5,7)
                                + "." + yearCalendarItem.currentDate.substring(0, 4)

                        view.globalDate = daySelected + "." + monthSelected + "." + yearSelected

                        yearCalendarItem.dateWasChosen()
                    }

                    style: CalendarStyle
                    {

                        navigationBar: Rectangle {
                            width: 300 - 4
                            height: 20

                            Text {
                                color: "#1563A5"
                                x: parent.width / 2 - width / 2
                                text: "<b>" + styleData.title + "</b>"
                            }
                        }

                    /*
                    dayOfWeekDelegate: Text
                    {
                        color: "#1563A5"
                        text:   Qt.locale().weekDays[styleData.index]
                    }
                    */
                        dayDelegate: Item
                        {
                            readonly property color sameMonthDateTextColor: "#1563A5"
                            readonly property color selectedDateColor: Qt.platform.os === "osx" ? "#3778d0" : systemPalette.highlight
                            readonly property color selectedDateTextColor: "white"
                            readonly property color differentMonthDateTextColor: "#bbb"
                            readonly property color invalidDatecolor: "#dddddd"

                            Rectangle {
                                anchors.fill: parent
                                border.color: "transparent"
                                color: styleData.date !== undefined
                                       && styleData.date.getDate() === view.lastSelectedDate.getDate()
                                       && styleData.date.getMonth() === view.lastSelectedDate.getMonth() ? selectedDateColor : "transparent"
                                anchors.margins: styleData.selected ? -1 : 0
                                //old condition: styleData.date !== undefined && styleData.selected ? "transparent" : "transparent" //selectedDateColor
                            }

                            /*
                            Image {
                                visible: paintedCalendar.checkDateIsForSet(styleData.date)
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.margins: -1
                                width: 12
                                height: width
                                source: "qrc:/icons/calendarconer.png"
                            }*/

                            Label
                            {
                                id: dayDelegateText
                                text: styleData.date.getDate()
                                anchors.centerIn: parent
                                color: {
                                    var color = invalidDatecolor
                                    if (styleData.valid)
                                    {
                                        color = styleData.visibleMonth ? sameMonthDateTextColor : differentMonthDateTextColor

                                        if (styleData.selected) {
                                            //color = selectedDateTextColor;
                                        }

                                        //if (styleData.date.getDate() === 3)
                                            //color = "#FF3366" //way to select

                                        if (paintedCalendar.checkDateBirthEmpty(styleData.date))
                                            color = "#BBBBBB"
                                    }
                                    dayDelegateText.color = color
                                }
                            } //Label
                        } //dayItem
                    } //CalendarStyle
                }//Calendar
            }
        } //Rectangle
    } //Component
}
