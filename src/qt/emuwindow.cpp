#include <cmath>
#include <fstream>
#include <iostream>

#include <QApplication>
#include <QString>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidget>
#include <QStatusBar>
#include <QStyleFactory> 

#include "emuwindow.hpp"
#include "settingswindow.hpp"
#include "renderwidget.hpp"
#include "gamelistwidget.hpp"
#include "bios.hpp"

#include "arg.h"

using namespace std;

EmuWindow::EmuWindow(QWidget *parent) : QMainWindow(parent)
{
    old_frametime = chrono::system_clock::now();
    framerate_avg = 0.0;
    frametime_avg = 0.016;

    for (int i = 0; i < 60; i++)
        frametime_list[i] = 0.016;

    render_widget = new RenderWidget;

    connect(&emu_thread, &EmuThread::completed_frame,
        render_widget, &RenderWidget::draw_frame
    );

    GameListWidget* game_list_widget = new GameListWidget;
    connect(game_list_widget, &GameListWidget::game_double_clicked, this, [=](QString path) {
        if (!path.isEmpty())
        {
            Settings::instance().add_rom_path(path);
            load_exec(path.toLocal8Bit(), true);
        }
    });

    connect(game_list_widget, &GameListWidget::settings_requested, [=]() {
        open_settings_window();
        settings_window->show_path_tab();
    });

    stack_widget = new QStackedWidget;
    stack_widget->addWidget(game_list_widget);
    stack_widget->addWidget(render_widget);

    int current_factor = Settings::instance().scaling_factor;

    stack_widget->setMinimumSize(
        RenderWidget::DEFAULT_WIDTH * current_factor,
        RenderWidget::DEFAULT_HEIGHT * current_factor
    );

    setCentralWidget(stack_widget);

    ee_mode = new QLabel;
    vu0_mode = new QLabel;
    vu1_mode = new QLabel;

    frametime = new QLabel;
    avg_framerate = new QLabel;

    update_status();

    connect(&Settings::instance(), &Settings::reload, [=]() {
        update_status();
    });

    statusBar()->addPermanentWidget(ee_mode);
    statusBar()->addPermanentWidget(vu0_mode);
    statusBar()->addPermanentWidget(vu1_mode);

    create_menu();

    connect(this, SIGNAL(shutdown()), &emu_thread, SLOT(shutdown()));
    connect(this, SIGNAL(press_key(sio2::PAD_BUTTON)), &emu_thread, SLOT(press_key(sio2::PAD_BUTTON)));
    connect(this, SIGNAL(release_key(sio2::PAD_BUTTON)), &emu_thread, SLOT(release_key(sio2::PAD_BUTTON)));
    connect(this, SIGNAL(update_joystick(sio2::JOYSTICK, sio2::JOYSTICK_AXIS, uint8_t)),
        &emu_thread, SLOT(update_joystick(sio2::JOYSTICK, sio2::JOYSTICK_AXIS, uint8_t))
    );
    connect(&emu_thread, SIGNAL(update_FPS(double)), this, SLOT(update_FPS(double)));
    connect(&emu_thread, SIGNAL(emu_error(QString)), this, SLOT(emu_error(QString)));
    connect(&emu_thread, SIGNAL(emu_non_fatal_error(QString)), this, SLOT(emu_non_fatal_error(QString)));
    connect(&emu_thread, &EmuThread::rom_loaded, this, [=](QString name, QString serial) {

        QFileInfo file_info(name);

        if (!serial.isEmpty())
            setWindowTitle(QString("[%1] %2").arg(
                serial, file_info.fileName()
            ));
        else
            setWindowTitle(file_info.fileName());
    });

    emu_thread.pause(PAUSE_EVENT::GAME_NOT_LOADED);

    emu_thread.reset();
    emu_thread.start();

    //Initialize window
    show_default_view();

    show();

    stack_widget->setMinimumSize(
        RenderWidget::DEFAULT_WIDTH,
        RenderWidget::DEFAULT_HEIGHT
    );
}

int EmuWindow::init(int argc, char** argv)
{
    bool skip_BIOS = false;
    char* argv0; // Program name; AKA argv[0]

    char* bios_name = nullptr, *file_name = nullptr, *gsdump = nullptr;

    ARGBEGIN {
        case 'b':
            bios_name = ARGF();
            Settings::instance().set_bios_path(
                QString::fromLocal8Bit(bios_name)
            );
            break;
        case 'f':
            file_name = ARGF();
            break;
        case 's':
            skip_BIOS = true;
            break;
        case 'g':
            gsdump = ARGF();
            break;
        case 'h':
        default:
            printf("usage: %s [options]\n\n", argv0);
            printf("options:\n");
            printf("-b {BIOS}\tspecify BIOS\n");
            printf("-f {ELF/ISO}\tspecify ELF/ISO\n");
            printf("-h\t\tshow this message\n");
            printf("-s\t\tskip BIOS\n");
            printf("-g {.GSD}\t\trun a gsdump\n");
            return 1;
    } ARGEND

    if (gsdump)
    {
        return load_exec(gsdump, false);
    }

    if (file_name)
    {
        if (load_exec(file_name, skip_BIOS))
            return 1;
    }

    Settings::instance().save();

    return 0;
}

EmuWindow::~EmuWindow()
{
    emu_thread.wait();
}


int EmuWindow::load_exec(const char* file_name, bool skip_BIOS)
{
    if (!load_bios())
        return 1;

    QFileInfo file_info(file_name);

    if (!file_info.exists())
    {
        printf("Failed to load %s\n", file_name);
        return 1;
    }

    if (!Settings::instance().memcard_path.isEmpty())
        emu_thread.load_memcard(0, Settings::instance().memcard_path.toStdString().c_str());

    QString ext = file_info.suffix();
    if(QString::compare(ext, "elf", Qt::CaseInsensitive) == 0)
    {
        QFile exec_file(file_name);
        if (!exec_file.open(QIODevice::ReadOnly))
        {
            QString path = file_info.absoluteFilePath();
            printf("Couldn't open %s\n", qPrintable(path));
            return 1;
        }

        QByteArray file_data(exec_file.readAll());

        emu_thread.load_ELF(
            file_name,
            reinterpret_cast<uint8_t *>(file_data.data()),
            exec_file.size()
        );

        if (skip_BIOS)
            emu_thread.set_skip_BIOS_hack(core::SKIP_HACK::LOAD_ELF);
    }
    else if (QString::compare(ext, "iso", Qt::CaseInsensitive) == 0)
    {
        emu_thread.load_CDVD(file_name, cdvd::CDVD_CONTAINER::ISO);
        if (skip_BIOS)
            emu_thread.set_skip_BIOS_hack(core::SKIP_HACK::LOAD_DISC);
    }
    else if (QString::compare(ext, "cso", Qt::CaseInsensitive) == 0)
    {
        emu_thread.load_CDVD(file_name, cdvd::CDVD_CONTAINER::CISO);
        if (skip_BIOS)
            emu_thread.set_skip_BIOS_hack(core::SKIP_HACK::LOAD_DISC);
    }
    else if (QString::compare(ext, "bin", Qt::CaseInsensitive) == 0)
    {
        emu_thread.load_CDVD(file_name, cdvd::CDVD_CONTAINER::BIN_CUE);
        if (skip_BIOS)
            emu_thread.set_skip_BIOS_hack(core::SKIP_HACK::LOAD_DISC);
    }
    else if (QString::compare(ext, "gsd", Qt::CaseInsensitive) == 0)
        emu_thread.gsdump_read(file_name);
    else
    {
        printf("Unrecognized file format %s\n", qPrintable(file_info.suffix()));
        return 1;
    }

    current_ROM = file_info;
    emu_thread.unpause(PAUSE_EVENT::GAME_NOT_LOADED);
    show_render_view();

    return 0;
}

void EmuWindow::create_menu()
{
    load_rom_action = new QAction(tr("Load ROM... (&Fast)"), this);
    connect(load_rom_action, &QAction::triggered, this, &EmuWindow::open_file_skip);

    load_bios_action = new QAction(tr("Load ROM... (&Boot BIOS)"), this);
    connect(load_bios_action, &QAction::triggered, this, &EmuWindow::open_file_no_skip);

    auto load_gsdump_action = new QAction(tr("Load &GSDump..."), this);
    connect(load_gsdump_action, &QAction::triggered, this, [=] (){
        emu_thread.pause(PAUSE_EVENT::FILE_DIALOG);

        QString file_name = QFileDialog::getOpenFileName(
            this, tr("Open Rom"),
            Settings::instance().last_used_directory,
            tr("GSDumps (*.gsd)")
        );

        if (!file_name.isEmpty())
        {
            Settings::instance().add_rom_path(file_name);
            load_exec(file_name.toLocal8Bit(), false);

            show_render_view();
        }

        emu_thread.unpause(PAUSE_EVENT::FILE_DIALOG);
    });

    load_state_action = new QAction(tr("&Load State"), this);
    connect(load_state_action, &QAction::triggered, this, &EmuWindow::load_state);

    save_state_action = new QAction(tr("&Save State"), this);
    connect(save_state_action, &QAction::triggered, this, &EmuWindow::save_state);

    auto toggle_gsdump_action = new QAction(tr("GS dump &toggle"), this);
    connect(toggle_gsdump_action, &QAction::triggered, this,
        [this]() { this->emu_thread.gsdump_write_toggle(); });

    exit_action = new QAction(tr("&Exit"), this);
    connect(exit_action, &QAction::triggered, this, &QWidget::close);

    file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(load_rom_action);
    file_menu->addAction(load_bios_action);
    file_menu->addAction(load_gsdump_action);

    auto recent_menu = file_menu->addMenu(tr("&Recent"));
    auto default_action = new QAction(tr("No recent roms..."));
    default_action->setEnabled(false);

    if (Settings::instance().recent_roms.isEmpty())
    {
        recent_menu->addAction(default_action);
    }

    for (auto& recent_file : Settings::instance().recent_roms)
    {
        auto recent_item_action = new QAction(recent_file);
        connect(recent_item_action, &QAction::triggered, this, [=]() {
            load_exec(recent_item_action->text().toLocal8Bit(), true);
        });
        recent_menu->addAction(recent_item_action);
    }

    auto clear_action = new QAction(tr("Clear List"));
    connect(clear_action, &QAction::triggered, this, [=]() {
        Settings::instance().clear_rom_paths();
        for (auto& old_action : recent_menu->actions())
        {
            recent_menu->removeAction(old_action);
        }

        recent_menu->addAction(default_action);
        recent_menu->addSeparator();
        recent_menu->addAction(clear_action);
    });

    connect(&Settings::instance(), &Settings::rom_path_added, this, [=](QString path) {
        recent_menu->clear();

        for (auto& recent_file : Settings::instance().recent_roms)
        {
            auto recent_item_action = new QAction(recent_file);
            connect(recent_item_action, &QAction::triggered, this, [=]() {
                load_exec(recent_item_action->text().toLocal8Bit(), true);
            });

            recent_menu->addAction(recent_item_action);
        }

        if (recent_menu->actions().contains(default_action))
            recent_menu->removeAction(default_action);

        recent_menu->addSeparator();
        recent_menu->addAction(clear_action);
    });

    recent_menu->addSeparator();
    recent_menu->addAction(clear_action);

    file_menu->addMenu(recent_menu);
    file_menu->addSeparator();
    file_menu->addAction(load_state_action);
    file_menu->addAction(save_state_action);
    file_menu->addSeparator();
    file_menu->addAction(toggle_gsdump_action);
    file_menu->addSeparator();
    file_menu->addAction(exit_action);

    auto memcard_window_action = new QAction(tr("&Memcards"), this);
    connect(memcard_window_action, &QAction::triggered, this, &EmuWindow::open_memcard_window);

    auto pause_action = new QAction(tr("&Pause"), this);
    connect(pause_action, &QAction::triggered, this, [=] (){
        emu_thread.pause(PAUSE_EVENT::USER_REQUESTED);
    });

    auto unpause_action = new QAction(tr("&Unpause"), this);
    connect(unpause_action, &QAction::triggered, this, [=] (){
        emu_thread.unpause(PAUSE_EVENT::USER_REQUESTED);
    });

    auto frame_action = new QAction(tr("&Frame Advance"), this);
    frame_action->setCheckable(true);
    connect(frame_action, &QAction::triggered, this, [=] (){
        emu_thread.frame_advance = emu_thread.frame_advance ^ true;

        if(!emu_thread.frame_advance)
            emu_thread.unpause(PAUSE_EVENT::FRAME_ADVANCE);

        frame_action->setChecked(emu_thread.frame_advance);
    });

    auto wavoutput_action = new QAction(tr("&WAV Audio Output"), this);
    wavoutput_action->setCheckable(true);
    connect(wavoutput_action, &QAction::triggered, this, [=] (){
        bool new_state = wavoutput_action->isChecked();
        emu_thread.set_wavout(new_state);
        wavoutput_action->setChecked(new_state);
    });


    auto shutdown_action = new QAction(tr("&Shutdown"), this);
    connect(shutdown_action, &QAction::triggered, this, [=]() {
        emu_thread.pause(PAUSE_EVENT::GAME_NOT_LOADED);
        show_default_view();
    });

    emulation_menu = menuBar()->addMenu(tr("Emulation"));
    emulation_menu->addAction(memcard_window_action);
    emulation_menu->addSeparator();
    emulation_menu->addAction(pause_action);
    emulation_menu->addAction(unpause_action);
    emulation_menu->addSeparator();
    emulation_menu->addAction(frame_action);
    emulation_menu->addAction(wavoutput_action);
    emulation_menu->addSeparator();
    emulation_menu->addAction(shutdown_action);

    auto settings_action = new QAction(tr("&Settings"), this);
    connect(settings_action, &QAction::triggered, this, &EmuWindow::open_settings_window);

    options_menu = menuBar()->addMenu(tr("&Options"));
    options_menu->addAction(settings_action);

    auto ignore_aspect_ratio_action =
    new QAction(tr("&Ignore aspect ratio"), this);
    ignore_aspect_ratio_action->setCheckable(true);

    connect(ignore_aspect_ratio_action, &QAction::triggered, render_widget, [=] (){
        render_widget->toggle_aspect_ratio();
        ignore_aspect_ratio_action->setChecked(
            !render_widget->get_respect_aspect_ratio()
        );
    });

    window_menu = menuBar()->addMenu(tr("&Window"));
    window_menu->addAction(ignore_aspect_ratio_action);
    window_menu->addSeparator();

    for (int factor = 1; factor <= RenderWidget::MAX_SCALING; factor++)
    {
        auto scale_action = new QAction(
            QString("Scale &%1x").arg(factor), this
        );

        connect(scale_action, &QAction::triggered, this, [=]() {
            // Force the widget to the new size
            stack_widget->setMinimumSize(
                RenderWidget::DEFAULT_WIDTH * factor,
                RenderWidget::DEFAULT_HEIGHT * factor
            );

            showNormal();
            adjustSize();

            // reset it so the user can resize the window
            // normally
            stack_widget->setMinimumSize(
                RenderWidget::DEFAULT_WIDTH,
                RenderWidget::DEFAULT_HEIGHT
            );

            Settings::instance().scaling_factor = factor;
            Settings::instance().save();
        });

        window_menu->addAction(scale_action);
    }

    auto screenshot_action = new QAction(tr("&Take Screenshot"), this);
    connect(screenshot_action, &QAction::triggered, render_widget, &RenderWidget::screenshot);

    window_menu->addSeparator();
    window_menu->addAction(screenshot_action);
}

bool EmuWindow::load_bios()
{
    BiosReader bios_file(Settings::instance().bios_path);
    if (!bios_file.is_valid())
    {
        bios_error(bios_file.to_string());

        return false;
    }

    emu_thread.load_BIOS(bios_file.data());

    return true;
}

void EmuWindow::open_settings_window()
{
    if (!settings_window)
        settings_window = new SettingsWindow(this);

    settings_window->show();
    settings_window->raise();
}

void EmuWindow::open_memcard_window()
{
    if (!memcard_window)
        memcard_window = new MemcardWindow(this);

    memcard_window->show();
    memcard_window->raise();
}

void EmuWindow::closeEvent(QCloseEvent *event)
{
    emit shutdown();
    event->accept();
}

void EmuWindow::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    switch (event->key())
    {
        case Qt::Key_Up:
            emit press_key(sio2::PAD_BUTTON::UP);
            break;
        case Qt::Key_Down:
            emit press_key(sio2::PAD_BUTTON::DOWN);
            break;
        case Qt::Key_Left:
            emit press_key(sio2::PAD_BUTTON::LEFT);
            break;
        case Qt::Key_Right:
            emit press_key(sio2::PAD_BUTTON::RIGHT);
            break;
        case Qt::Key_Z:
            emit press_key(sio2::PAD_BUTTON::CROSS);
            break;
        case Qt::Key_X:
            emit press_key(sio2::PAD_BUTTON::CIRCLE );
            break;
        case Qt::Key_A:
            emit press_key(sio2::PAD_BUTTON::SQUARE);
            break;
        case Qt::Key_S:
            emit press_key(sio2::PAD_BUTTON::TRIANGLE );
            break;
        case Qt::Key_Q:
            emit press_key(sio2::PAD_BUTTON::L1);
            break;
        case Qt::Key_W:
            emit press_key(sio2::PAD_BUTTON::R1);
            break;
        case Qt::Key_Return:
            emit press_key(sio2::PAD_BUTTON::START);
            break;
        case Qt::Key_Space:
            emit press_key(sio2::PAD_BUTTON::SELECT);
            break;
        case Qt::Key_Period:
            emu_thread.unpause(PAUSE_EVENT::FRAME_ADVANCE);
            break;
        case Qt::Key_R:
            emit update_joystick(sio2::JOYSTICK::LEFT, sio2::JOYSTICK_AXIS::Y, 0x00);
            break;
        case Qt::Key_F:
            emit update_joystick(sio2::JOYSTICK::LEFT, sio2::JOYSTICK_AXIS::Y, 0xFF);
            break;
        case Qt::Key_G:
            emit update_joystick(sio2::JOYSTICK::LEFT, sio2::JOYSTICK_AXIS::X, 0xFF);
            break;
        case Qt::Key_D:
            emit update_joystick(sio2::JOYSTICK::LEFT, sio2::JOYSTICK_AXIS::X, 0x00);
            break;
        case Qt::Key_I:
            emit update_joystick(sio2::JOYSTICK::RIGHT, sio2::JOYSTICK_AXIS::Y, 0x00);
            break;
        case Qt::Key_K:
            emit update_joystick(sio2::JOYSTICK::RIGHT, sio2::JOYSTICK_AXIS::Y, 0xFF);
        case Qt::Key_J:
            emit update_joystick(sio2::JOYSTICK::RIGHT, sio2::JOYSTICK_AXIS::X, 0xFF);
            break;
        case Qt::Key_L:
             emit update_joystick(sio2::JOYSTICK::RIGHT, sio2::JOYSTICK_AXIS::X, 0x00);
             break;
        case Qt::Key_F1:
            if(!Settings::instance().recent_roms.isEmpty())
                load_exec(Settings::instance().recent_roms.first().toLocal8Bit(), true);
            break;
        case Qt::Key_F2:
            if(!Settings::instance().recent_roms.isEmpty())
            {
                load_exec(Settings::instance().recent_roms.first().toLocal8Bit(), true);
                load_state();
            }
            break;
        case Qt::Key_F7:
            emu_thread.gsdump_single_frame();
            break;
        case Qt::Key_F8:
            render_widget->screenshot();
            break;
    }
}

void EmuWindow::keyReleaseEvent(QKeyEvent *event)
{
    event->accept();
    switch (event->key())
    {
        case Qt::Key_Up:
            emit release_key(sio2::PAD_BUTTON::UP);
            break;
        case Qt::Key_Down:
            emit release_key(sio2::PAD_BUTTON::DOWN);
            break;
        case Qt::Key_Left:
            emit release_key(sio2::PAD_BUTTON::LEFT);
            break;
        case Qt::Key_Right:
            emit release_key(sio2::PAD_BUTTON::RIGHT);
            break;
        case Qt::Key_Z:
            emit release_key(sio2::PAD_BUTTON::CROSS);
            break;
        case Qt::Key_X:
            emit release_key(sio2::PAD_BUTTON::CIRCLE );
            break;
        case Qt::Key_A:
            emit release_key(sio2::PAD_BUTTON::SQUARE);
            break;
        case Qt::Key_S:
            emit release_key(sio2::PAD_BUTTON::TRIANGLE);
            break;
        case Qt::Key_Q:
            emit release_key(sio2::PAD_BUTTON::L1);
            break;
        case Qt::Key_W:
            emit release_key(sio2::PAD_BUTTON::R1);
            break;
        case Qt::Key_Return:
            emit release_key(sio2::PAD_BUTTON::START);
            break;
        case Qt::Key_Space:
            emit release_key(sio2::PAD_BUTTON::SELECT);
            break;
        case Qt::Key_R:
        case Qt::Key_F:
            emit update_joystick(sio2::JOYSTICK::LEFT, sio2::JOYSTICK_AXIS::Y, 0x80);
            break;
        case Qt::Key_D:
        case Qt::Key_G:
            emit update_joystick(sio2::JOYSTICK::LEFT, sio2::JOYSTICK_AXIS::X, 0x80);
            break;
        case Qt::Key_I:
        case Qt::Key_K:
            emit update_joystick(sio2::JOYSTICK::RIGHT, sio2::JOYSTICK_AXIS::Y, 0x80);
            break;
        case Qt::Key_J:
        case Qt::Key_L:
            emit update_joystick(sio2::JOYSTICK::RIGHT, sio2::JOYSTICK_AXIS::X,0x80);
            break;
    }
}

void EmuWindow::update_FPS(double FPS)
{
    if(FPS > 0.01) {
        frametime_list[frametime_list_index] = 1. / FPS;
        frametime_list_index = (frametime_list_index + 1) % 60;
    }

    double worst_frame_time = 0;
    frametime_avg = 0;

    for(int i = 0; i < 60; i++)
    {
        if(frametime_list[i] > worst_frame_time) worst_frame_time = frametime_list[i];
        frametime_avg += frametime_list[i];
    }

    if (frametime_avg > 0)
    {
        frametime_avg = frametime_avg / 60.0;
        framerate_avg = 1.0 / frametime_avg;
    }
    else
        framerate_avg = 0;

    avg_framerate->setText(QString("%1 fps").arg(
        QString::number(framerate_avg, 'f', 1)
    ));

    frametime->setText(QString("%1 ms / %2 ms").arg(
        QString::number(frametime_avg * 1000., 'f', 1),
        QString::number(worst_frame_time * 1000., 'f', 1)
    ));
}

void EmuWindow::bios_error(QString err)
{
    QMessageBox msg_box;
    msg_box.setText("Emulation has been terminated");
    msg_box.setInformativeText(err);
    msg_box.setStandardButtons(QMessageBox::Abort);
    msg_box.setDefaultButton(QMessageBox::Abort);
    msg_box.exec();
}

void EmuWindow::emu_error(QString err)
{
    QMessageBox msgBox;
    msgBox.setText("Emulation has been terminated");
    msgBox.setInformativeText(err);
    msgBox.setStandardButtons(QMessageBox::Abort);
    msgBox.setDefaultButton(QMessageBox::Abort);
    msgBox.exec();
    current_ROM = QFileInfo();
    show_default_view();
}

void EmuWindow::emu_non_fatal_error(QString err)
{
    QMessageBox msgBox;
    msgBox.setText("Error");
    msgBox.setInformativeText(err);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
    emu_thread.unpause(MESSAGE_BOX);
}

#ifndef QT_NO_CONTEXTMENU
void EmuWindow::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    menu.addAction(load_rom_action);
    menu.addAction(load_bios_action);
    menu.addAction(exit_action);
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU

void EmuWindow::open_file_no_skip()
{
    emu_thread.pause(PAUSE_EVENT::FILE_DIALOG);
    QString file_name = QFileDialog::getOpenFileName(
        this, tr("Open Rom"), Settings::instance().last_used_directory,
        tr("ROM Files (*.elf *.iso *.cso *.bin *.chd)")
    );

    if (!file_name.isEmpty())
    {
        Settings::instance().add_rom_path(file_name);
        load_exec(file_name.toStdString().c_str(), false);
    }

    emu_thread.unpause(PAUSE_EVENT::FILE_DIALOG);
}

void EmuWindow::open_file_skip()
{
    emu_thread.pause(PAUSE_EVENT::FILE_DIALOG);
    QString file_name = QFileDialog::getOpenFileName(
        this, tr("Open Rom"), Settings::instance().last_used_directory,
        tr("ROM Files (*.elf *.iso *.cso *.bin *.chd)")
    );

    if (!file_name.isEmpty())
    {
        Settings::instance().add_rom_path(file_name);
        load_exec(file_name.toStdString().c_str(), true);
    }

    emu_thread.unpause(PAUSE_EVENT::FILE_DIALOG);
}

void EmuWindow::load_state()
{
    emu_thread.pause(PAUSE_EVENT::FILE_DIALOG);
    QString file_name = current_ROM.baseName();
    QString directory = current_ROM.absoluteDir().path();

    QString save_state = directory
        .append(QDir::separator())
        .append(file_name)
        .append(".snp");

    if (!QFile::exists(save_state) || !emu_thread.load_state(save_state.toLocal8Bit()))
        printf("Failed to load %s\n", qPrintable(save_state));

    emu_thread.unpause(PAUSE_EVENT::FILE_DIALOG);
}

void EmuWindow::save_state()
{
    emu_thread.pause(PAUSE_EVENT::FILE_DIALOG);
    QString file_name = current_ROM.baseName();
    QString directory = current_ROM.absoluteDir().path();

    QString save_state = directory
        .append(QDir::separator())
        .append(file_name)
        .append(".snp");

    if (!emu_thread.save_state(save_state.toLocal8Bit()))
        printf("Failed to save %s\n", qPrintable(save_state));
    emu_thread.unpause(PAUSE_EVENT::FILE_DIALOG);
}

void EmuWindow::show_default_view()
{
    statusBar()->removeWidget(avg_framerate);
    statusBar()->removeWidget(frametime);

    stack_widget->setCurrentIndex(0);
    setWindowTitle(QApplication::applicationName());
}

void EmuWindow::show_render_view()
{
    statusBar()->addWidget(avg_framerate);
    statusBar()->addWidget(frametime);

    avg_framerate->show();
    frametime->show();

    stack_widget->setCurrentIndex(1);
}

void EmuWindow::update_status()
{
    core::CPU_MODE mode;

    if (Settings::instance().d_theme) // Dark Theme colour change  
    {
        qApp->setStyle(QStyleFactory::create("Fusion"));
        QPalette darkPalette;
        QColor darkColor = QColor(35,39,42);
        QColor disabledColor = QColor(200,45,69);
        darkPalette.setColor(QPalette::Window, darkColor);
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(35,39,42));
        darkPalette.setColor(QPalette::AlternateBase, darkColor);
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
        darkPalette.setColor(QPalette::Button, darkColor);
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(200,45,69));
        darkPalette.setColor(QPalette::Highlight, QColor(200,45,69));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
        qApp->setPalette(darkPalette);
        qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    }
    
    if (Settings::instance().l_theme)// Change to  Light Theme   
    {
        qApp->setStyle(QStyleFactory::create("Fusion"));
        QPalette lightPalette;
        QColor lightColor = QColor(255,255,255);
        qApp->setPalette(lightPalette);
        lightPalette.setColor(QPalette::Window, lightColor);
        lightPalette.setColor(QPalette::WindowText, Qt::black);
        lightPalette.setColor(QPalette::Base, QColor(255,255,255));
        lightPalette.setColor(QPalette::AlternateBase, lightColor);
        lightPalette.setColor(QPalette::ToolTipBase, Qt::white);
        lightPalette.setColor(QPalette::ToolTipText, Qt::black);
        lightPalette.setColor(QPalette::Text, Qt::black);
        lightPalette.setColor(QPalette::Button, lightColor);
        lightPalette.setColor(QPalette::ButtonText, Qt::black);
        lightPalette.setColor(QPalette::BrightText, Qt::black);
        lightPalette.setColor(QPalette::Link, QColor(98, 102, 102));
        lightPalette.setColor(QPalette::Highlight, QColor(214, 214, 214));
        qApp->setPalette(lightPalette);
        qApp->setStyleSheet("");    
    }

    if (Settings::instance().ee_jit_enabled)
    {
        mode = core::CPU_MODE::JIT;
        ee_mode->setText("EE: JIT");
    }
    else
    {
        mode = core::CPU_MODE::INTERPRETER;
        ee_mode->setText("EE: Interpreter");
    }

    emu_thread.set_ee_mode(mode);

    if (Settings::instance().vu0_jit_enabled)
    {
        mode = core::CPU_MODE::JIT;
        vu0_mode->setText("VU0: JIT");
    }
    else
    {
        mode = core::CPU_MODE::INTERPRETER;
        vu0_mode->setText("VU0: Interpreter");
    }
    emu_thread.set_vu0_mode(mode);

    if (Settings::instance().vu1_jit_enabled)
    {
        mode = core::CPU_MODE::JIT;
        vu1_mode->setText("VU1: JIT");
    }
    else
    {
        mode = core::CPU_MODE::INTERPRETER;
        vu1_mode->setText("VU1: Interpreter");
    }
    emu_thread.set_vu1_mode(mode);
}
