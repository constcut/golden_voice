import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.4 as Quick1

Item {
    id: recorderItem

    function fillListWithRecords(audios, texts)
    {
        rowsModel.clear()

        var merged = []

        if (audios !== undefined)
            for (var i = 0; i < audios.length; ++i)
            {
                var audio = audios[i]
                merged.push({"name":audio[3], "date": audio[0],
                                    "time": audio[1], "id": audio[2], "duration": audio[4],
                                    "tags": audio[5], "description": audio[6], "rowType":"audio"});
            }

        if (texts !== undefined)
            for (i = 0; i < texts.length; ++i)
            {
                var text = texts[i]
                merged.push({"name":text[3], "date": text[0],
                                    "time": text[1], "id": text[2], "textValue": text[4],
                                    "tags": text[5], "description": text[6], "rowType":"text"});
            }

        merged.sort(function(lhs, rhs) {
            if(lhs.time < rhs.time) { return -1; }
            if(lhs.time > rhs.time) { return 1; }
            return 0;
        })


        for (i = 0; i < merged.length; ++i)
            rowsModel.append(merged[i])
    }


    ColumnLayout
    {
        spacing: 10

        Text {
            id: currentDateText
            text: "Date"
            visible: false
        }

        RowLayout
        {
            spacing: 10

            Quick1.Calendar
            {
                id: calendar

                onSelectedDateChanged:
                {
                    currentDateText.text = selectedDate //Walkaround
                    currentDateText.text = currentDateText.text.substring(0, 10)
                    searchBox.search()
                }
            }

            TextField {
                id: searchBox
                placeholderText: "Search string"

                onEditingFinished: {
                    search()
                }

                function search()
                {
                    var needAudio = false
                    var needText = false

                    if (searchAudio.checked)
                        needAudio = true

                    if (searchTexts.checked)
                        needText = true

                    if (searchAll.checked)
                    {
                        needAudio = true
                        needText = true
                    }

                    var foundAudios = []
                    var foundTexts = []

                    if (useDateInSearch.checked)
                    {
                        if (searchByName.checked)
                        {
                            if (needAudio)
                                foundAudios = sqlBase.findRecordsByNameMaskAndDate(
                                               currentDateText.text, searchBox.text)
                            if (needText)
                                foundTexts = sqlBase.findTextsByNameMaskAndDate(
                                           currentDateText.text, searchBox.text)
                        }
                        else
                        {
                            if (needAudio)
                                foundAudios = sqlBase.findRecordsByTagMaskAndDate(
                                               currentDateText.text, searchBox.text)
                            if (needText)
                                foundTexts = sqlBase.findTextsByTagMaskAndDate(
                                           currentDateText.text, searchBox.text)
                        }
                    }
                    else
                    {
                        if (searchByName.checked)
                        {
                            if (needAudio)
                                foundAudios = sqlBase.findRecordsByNameMask(searchBox.text)

                            if (needText)
                                foundTexts = sqlBase.findTextsByNameMask(searchBox.text)
                        }
                        else
                        {
                            if (needAudio)
                                foundAudios = sqlBase.findRecordsByTagMask(searchBox.text)

                            if (needText)
                                foundTexts = sqlBase.findTextsByNameMask(searchBox.text)
                        }
                    }

                    recorderItem.fillListWithRecords(foundAudios, foundTexts)
                    //вероятно хорошо бы выделять первый элемент, если доступ к его подфункциям будет происходить вне кликов по листу
                }
            }

            RadioButton {
                id: searchByName
                checked: true
                text: "name"
            }

            RadioButton {
                id: searchByTag
                text: "tag"
            }

            CheckBox {
                id: useDateInSearch
                text: "Use date in search"
                checked: true
            }

            ColumnLayout {
                RadioButton {
                    id: searchAudio
                    text: "Audio"
                }
                RadioButton {
                    id: searchTexts
                    text: "Texts"
                }
                RadioButton {
                    id: searchAll
                    text: "All"
                    checked: true
                }
            }


            Button {
                text: "Search"
                onClicked: {
                    searchBox.search()
                }
            }

            ColumnLayout {

                Button {
                    text: "+Audio"
                    onClicked:  {
                        mainWindow.requestAddAudioRecord()
                    }
                }

                Button {
                    text: "+Text"
                    onClicked:  {
                        mainWindow.requestAddText()
                    }
                }
            }

        }


        ListModel {
            id: rowsModel

            property string lastDate: ""
            property int lastLocalId: 0
        }


        Rectangle
        {
            id: listViewRectangle
            width: 600
            height: 300
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
                    text: "Audio: " + name + "   " + date + " T " + time + " " + duration + " ms"
                    visible: rowType === "audio"
                }
                Text {
                    text: "Text: " + name + "  " + date + " T " + time
                    visible: rowType === "text"
                }

                Text {
                    text: tags + " " + description
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
                    rowsModel.lastDate = date
                    rowsModel.lastLocalId = id
                }
                onDoubleClicked:
                {
                    if (rowType === "audio")
                        mainWindow.requestSingleRecord(rowsModel.lastDate, rowsModel.lastLocalId)

                    if (rowType === "text")
                        mainWindow.requestSingleText(rowsModel.lastDate, rowsModel.lastLocalId)
                }
                onPressAndHold:
                {
                    wrapper.ListView.view.currentIndex = index
                    rowsModel.lastDate = date
                    rowsModel.lastLocalId = id

                    if (rowType === "audio")
                    {
                        audioMenu.x = mouseX
                        audioMenu.y = mouseY + listViewRectangle.y
                        audioMenu.open()
                    }
                    if (rowType === "text")
                    {
                        textsMenu.x = mouseX
                        textsMenu.y = mouseY + listViewRectangle.y
                        textsMenu.open()
                    }
                }
            }
        }
    }


    Menu
    {
        id: audioMenu
        MenuItem {
            text: "Open"
            onTriggered: {
                 mainWindow.requestSingleRecord(rowsModel.lastDate, rowsModel.lastLocalId)
            }
        }
        MenuItem {
            text: "Play"
            onTriggered: {
                recorder.playFile(rowsModel.lastDate, rowsModel.lastLocalId)
            }
        }
        MenuItem {
            text: "Remove"
            onTriggered: {
                confirmDelete.type = "audio"
                confirmDelete.open()
            }
        }
    }


    Menu
    {
        id: textsMenu
        MenuItem {
            text: "Open"
            onTriggered: {
                mainWindow.requestSingleText(rowsModel.lastDate, rowsModel.lastLocalId)
            }
        }
        MenuItem {
            text: "Remove"
            onTriggered: {
                confirmDelete.type = "text"
                confirmDelete.open()
            }
        }
    }



    MessageDialog
    {
        property string type: ""

        id: confirmDelete
        title: "Delete file"
        text: "Do you really want to delete selected record?"
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {

            if (type == "audio")
                sqlBase.removeAudioRecord(rowsModel.lastDate, rowsModel.lastLocalId)

            if (type == "text")
                sqlBase.removeText(rowsModel.lastDate, rowsModel.lastLocalId)

            searchBox.search()
        }
        visible: false
    }



    Connections {
        target: recorder

        function onTimeUpdate(ms) {
            timeText.text = ms
        }

        function onDbsUpdate(dbs) {
            dbsText.text = dbs
        }
    }


    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
