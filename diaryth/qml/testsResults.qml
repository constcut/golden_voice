import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.4 as Quick1


Item {

    id: testsResulstsItem

    Component.onCompleted:
    {
        allTests.model = sqlBase.getAllTestsNames()
        loadResults()
    }

    function loadResults()
    {
        var resultsCount = sqlBase.getTestsResulstCount(allTests.currentText)
        var resultsList = sqlBase.getAllTestsResults(allTests.currentText)
        resultsRepeater.model = 0
        resultsRepeater.model = resultsCount

        for (var i = 0; i < resultsCount; ++i)
        {
            var currentResult = resultsList[i]
            resultsRepeater.itemAt(i).loadValues(currentResult[1], currentResult[2],
                                                 currentResult[3], currentResult[4])
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

            ComboBox {
                id: allTests
                implicitWidth: 150

                onCurrentTextChanged: {
                    testsResulstsItem.loadResults()
                }
            }

            RoundButton {
                text: "Back to tests"
                onClicked: mainWindow.requestTestsEngine()
            }
        }

        Repeater
        {
            id: resultsRepeater

            RowLayout
            {
                function loadValues(date, time, rate, rateInfo)
                {
                    dateText.text = date
                    timeText.text = time
                    rateText.text = rate
                    rateName.text = rateInfo
                }


                Text {
                    text: "Date: "
                }
                Text {
                    id: dateText
                }
                Text {
                    text: "Time: "
                }
                Text {
                    id: timeText
                }
                Text {
                    text: "Rate: "
                }
                Text {
                    id: rateText
                }
                Text {
                    text: "Rate name: "
                }
                Text {
                    id: rateName
                }
            }
        }


    } // main ColumnLayout


    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
