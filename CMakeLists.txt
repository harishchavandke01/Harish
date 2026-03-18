cmake_minimum_required(VERSION 3.16)

project(SurveyPod VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS PrintSupport Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport charts Svg)
find_package(Eigen3 CONFIG REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    find_package(Qt6 REQUIRED COMPONENTS Core Charts)
    find_package(Qt6 REQUIRED COMPONENTS Core)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    find_package(Qt6 REQUIRED COMPONENTS Core)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    find_package(Qt6 REQUIRED COMPONENTS Core)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport Charts Svg)
    find_package(Eigen3 CONFIG REQUIRED)

    qt_add_executable(SurveyPod
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Resources/resources.qrc
        Components/TitleBar/titlebar.h Components/TitleBar/titlebar.cpp
        Components/CreateProject/createproject.h Components/CreateProject/createproject.cpp
        Components/Workspace/workspace.h Components/Workspace/workspace.cpp
        Components/Workspace/RinexConversion/rinexconversion.h Components/Workspace/RinexConversion/rinexconversion.cpp
        Components/Workspace/BaselineProcessing/baselineprocessui.h Components/Workspace/BaselineProcessing/baselineprocessui.cpp
        Components/Utils/custommessagebox.h Components/Utils/custommessagebox.cpp
        Components/Utils/customcheckbox.h Components/Utils/customcheckbox.cpp


        Components/Utils/utils.h Components/Utils/utils.cpp
        Components/Workspace/RinexConversion/signalsmask.h Components/Workspace/RinexConversion/signalsmask.cpp

        Components/Utils/customprogressbar.h Components/Utils/customprogressbar.cpp
        Components/Utils/customloadingbar.h Components/Utils/customloadingbar.cpp

        backend/baselineprocessing.h backend/baselineprocessing.cpp
        backend/runbaselineprocessing.h backend/runbaselineprocessing.cpp
        backend/baselinewrapper.h backend/baselinewrapper.cpp
        Components/Utils/BaselineUtils/baselineutils.h Components/Utils/BaselineUtils/baselineutils.cpp
        Components/Utils/BaselineUtils/baselinedata.h Components/Utils/BaselineUtils/baselinedata.cpp
        Components/Workspace/BaselineProcessing/ChartView/chartview.h Components/Workspace/BaselineProcessing/ChartView/chartview.cpp
        Components/Utils/ProcessUtils/processutils.h Components/Utils/ProcessUtils/processutils.cpp
        Components/Workspace/BaselineProcessing/ProcessOptions/processoptions.h Components/Workspace/BaselineProcessing/ProcessOptions/processoptions.cpp
        Components/Workspace/BaselineProcessing/GenerateReport/generatereport.h Components/Workspace/BaselineProcessing/GenerateReport/generatereport.cpp
        Components/Workspace/BaselineProcessing/EditOptions/editoptions.h Components/Workspace/BaselineProcessing/EditOptions/editoptions.cpp
        Components/Workspace/NetworkAdjustment/networkadjustment.h Components/Workspace/NetworkAdjustment/networkadjustment.cpp
        Components/Context/projectcontext.h Components/Context/projectcontext.cpp
        Components/Workspace/NetworkAdjustment/FixStations/fixstations.h Components/Workspace/NetworkAdjustment/FixStations/fixstations.cpp
        Components/Workspace/NetworkAdjustment/AdjustOptions/adjustoptions.h Components/Workspace/NetworkAdjustment/AdjustOptions/adjustoptions.cpp
        Components/Workspace/BaselineProcessing/RawDataCheckIn/rawdatacheckin.h Components/Workspace/BaselineProcessing/RawDataCheckIn/rawdatacheckin.cpp
        Components/Workspace/BaselineProcessing/StationReferenceSelection/stationreferenceselection.h Components/Workspace/BaselineProcessing/StationReferenceSelection/stationreferenceselection.cpp
        Components/Workspace/BaselineProcessing/TimeBasedView/timebasedview.h Components/Workspace/BaselineProcessing/TimeBasedView/timebasedview.cpp
        Components/Utils/LoopClosureUtils/loopclosureutils.h Components/Utils/LoopClosureUtils/loopclosureutils.cpp
        Components/Workspace/BaselineProcessing/GenerateReport/generateloopclosurereport.h Components/Workspace/BaselineProcessing/GenerateReport/generateloopclosurereport.cpp
        Components/Workspace/NetworkAdjustment/SubnetworkUtils/subnetworkutils.h Components/Workspace/NetworkAdjustment/SubnetworkUtils/subnetworkutils.cpp
        Components/Workspace/NetworkAdjustment/AdjustNetworkDialog/adjustnetworkdialog.h Components/Workspace/NetworkAdjustment/AdjustNetworkDialog/adjustnetworkdialog.cpp

    )
else()
    if(ANDROID)
        add_library(SurveyPod SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(SurveyPod
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(SurveyPod PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(SurveyPod PRIVATE Qt6::Widgets)
target_link_libraries(SurveyPod PRIVATE Qt6::Core)
target_link_libraries(SurveyPod PRIVATE Qt6::Svg)
target_link_libraries(SurveyPod PRIVATE Qt6::PrintSupport)
target_link_libraries(SurveyPod PRIVATE Qt${QT_VERSION_MAJOR}::Charts)
target_link_libraries(SurveyPod PRIVATE Eigen3::Eigen)


target_include_directories(SurveyPod PRIVATE
    "${PROJECT_SOURCE_DIR}/components/utils/baselineUtils/proj")

target_link_libraries(SurveyPod PRIVATE
    "${PROJECT_SOURCE_DIR}/components/utils/baselineUtils/proj/proj.lib")


# RTKLIB
add_subdirectory(Backend/src)
target_link_libraries(SurveyPod PRIVATE rtklib)
target_link_libraries(SurveyPod PRIVATE Qt6::Widgets)
target_link_libraries(SurveyPod PRIVATE Qt6::Core)
target_link_libraries(SurveyPod PRIVATE Qt6::Widgets)
target_link_libraries(SurveyPod PRIVATE Qt6::Widgets)
target_link_libraries(SurveyPod PRIVATE Qt6::Widgets)
target_link_libraries(SurveyPod PRIVATE Qt6::Core)
target_link_libraries(SurveyPod PRIVATE Qt6::Widgets)
target_link_libraries(SurveyPod PRIVATE Qt6::Core)
target_link_libraries(SurveyPod PRIVATE Qt6::Widgets)
# RTKLIB


if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SurveyPod)
endif()
set_target_properties(SurveyPod PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SurveyPod
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SurveyPod)
endif()
