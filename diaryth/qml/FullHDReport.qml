import QtQuick 2.12
import diaryth 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1


Item {

    id: fullHDReport


    Component.onCompleted:
    {

    }


    JsonReport
    {
        id: jsonReport

        Component.onCompleted:
        {
            reloadVisualReports()
        }

    }

    function reloadVisualReports()
    {
        flick.contentWidth = jsonReport.getFullWidth()
        for (var i = 0; i < reportsRepeater.model; ++i)
        {
            reportsRepeater.itemAt(i).setParent(jsonReport)
            reportsRepeater.itemAt(i).width = jsonReport.getFullWidth()
            reportsRepeater.itemAt(i).setType(fullHDReport.reportsTypes[i])
        }
    }


    property var reportsHeight: [35, 200, 200, 500]
    property var reportsTypes: [VisualTypes.PlainWords, VisualTypes.PraatInfo, VisualTypes.ReportFields, VisualTypes.Pitch]


    function calculateY(idx)
    {
        var hSum = 0
        for (var i = 1; i <= idx; ++i)
            hSum += reportsHeight[i - 1]

        //console.log("ON idx ", idx, " we got ", hSum)

        return hSum + 5 * (idx + 1)
    }


    ScrollView
    {
        id: scroll
        width: parent.width
        height: fullHDReport.reportsHeight[0] + reportsHeight[1] + reportsHeight[2] + reportsHeight[3]

        Flickable
        {
            id: flick
            y: 5
            x: 0
            width: parent.width
            height: scroll.height
            contentWidth: 3000
            contentHeight:  parent.height
            property int pressedX : 0


            Repeater
            {
                id: reportsRepeater

                model: 4

                VisualReport
                {
                    id: visualReport
                    height:  fullHDReport.reportsHeight[index]
                    width: 3000
                    y:  fullHDReport.calculateY(index)

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked:{

                        }
                        onDoubleClicked: {
                            var idx = jsonReport.eventIdxOnClick(mouseX, mouseY)
                            jsonReport.selectEvent(idx)
                        }

                    }
                }


            }




        } //Flickable

    } //ScrollView




}
