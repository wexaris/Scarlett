#include "bump_reader.hpp"
#include "errors/errors.hpp"

UTFBumpReader::UTFBumpReader(const std::string& filepath) :
    reader(filepath)
{
    //next_cp = reader.read();
    //last_index = reader.get_index();
    //bump();

    curr_cp = reader.read();
    last_index = reader.get_index();
    next_cp = reader.read();
}

void UTFBumpReader::bump(uint n) {
    for (; n != 0; n--) {

        if (curr_cp == '\n') {
            text_pos.line++;
            text_pos.col = 1;
        }
        else {
            text_pos.col++;
        }

        curr_cp = next_cp;

        if (curr_cp != '\0') {
            text_pos.idx = get_index();
        }

        next_cp = reader.read();
    }
}