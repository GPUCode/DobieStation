#pragma once
#include "vlc_table.hpp"

class ChromTable : public VLC_Table
{
    private:
        static VLC_Entry table[];
        static unsigned int index_table[];

        constexpr static int SIZE = 12;
    public:
        ChromTable();
};
