import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.4 as Quick1


Item {

    id: cardRecordsItem

    Component.onCompleted:
    {
        allCards.model = sqlBase.getAllCardsNames()
        loadResults()
    }

    function loadResults()
    {
        var cardRecords

        if (useDateCheck.checked)
        {
            if (useCardCheck.checked)
                cardRecords = sqlBase.getAllCardRecordsForCardAndDate(
                              allCards.currentText, cardDateText.text)
            else
               cardRecords = sqlBase.getAllCardRecordsOnDate(cardDateText.text)
        }
        else
        {
            if (useCardCheck.checked)
                cardRecords = sqlBase.getAllCardRecordsForCard(allCards.currentText)
            else
                cardRecords = sqlBase.getAllCardRecords();

        }

        rowsModel.clear()

        for (var i = 0; i < cardRecords.length; ++i)
            rowsModel.append({"datePart":cardRecords[i][0], "timePart":cardRecords[i][1], "localId":cardRecords[i][2],
                             "cardId":cardRecords[i][3], "cardDate":cardRecords[i][4], "groupName":cardRecords[i][5],
                             "fieldName":cardRecords[i][6], "fieldValue":cardRecords[i][7], "fieldText":cardRecords[i][8]})
    }


    Dialog
    {
        id: calendarDialog

        width: 400
        height: 400

        Quick1.Calendar
        {
            anchors.fill: parent

            id: calendar

            onSelectedDateChanged:
            {
                cardDateText.text = selectedDate
                cardDateText.text = cardDateText.text.substring(0, 10)

                if (useCardCheck.checked)
                    cardRecordsItem.loadResults()
            }
        }
    }


    ColumnLayout
    {
        spacing: 10
        y: 40
        x: 40


        RowLayout
        {
            spacing: 10

            CheckBox {
                id: useCardCheck
                text: "Use card"
                checked: true

                onCheckedChanged: cardRecordsItem.loadResults()
            }

            ComboBox {
                id: allCards
                implicitWidth: 150

                onCurrentTextChanged: {

                    if (useCardCheck.checked)
                        cardRecordsItem.loadResults()
                }
            }

            CheckBox {
                id: useDateCheck
                text: "Use date"
                checked: true

                onCheckedChanged: cardRecordsItem.loadResults()
            }

            Text {
                text: "Card date: "
            }

            Text {
                id: cardDateText
            }

            RoundButton {
                text: ".."
                onClicked: calendarDialog.open()
            }

            RoundButton {
                text: "Back to cards"
                onClicked: mainWindow.requestDiaryCard()
            }
        }

        ListModel {
            id: rowsModel
        }

        Rectangle
        {
            id: listViewRectangle
            width: 900
            height: 400
            ListView
            {
                id: recordsList
                clip: true
                anchors.fill: parent
                model: rowsModel
                Behavior on y { NumberAnimation{ duration: 200 } }
                onContentYChanged: {} //When implement search bar copy behavior
                delegate: recordDeligate
                highlight: highlightBar
                focus:  true
                ScrollBar.vertical: ScrollBar {}
            }
        }


        Component {
            id: highlightBar
            Rectangle {
                id: highlightBarRect
                width: 200; height: 50
                color: "#88FF88"
                y: recordsList.currentItem == null ? 0 : recordsList.currentItem.y
                Behavior on y { SpringAnimation { spring: 2; damping: 0.3 } }
            }
        }



        Component {
            id: recordDeligate
            Item {
                id: wrapper
                width: recordsList.width
                height: 35
                Column {
                    Text {
                        text: datePart + " " + timePart + " " + cardDate + " " + groupName + " " + fieldName + " " + fieldValue + fieldText
                    }
                }
                states: State {
                    name: "Current"
                    when: wrapper.ListView.isCurrentItem
                    PropertyChanges { target: wrapper; x: 20 }
                }
                transitions: Transition {
                    NumberAnimation { properties: "x"; duration: 200 }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked:
                    {
                        wrapper.ListView.view.currentIndex = index
                    }
                }
            }
        }


    } // main ColumnLayout


    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
