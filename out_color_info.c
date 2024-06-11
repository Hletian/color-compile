/*
 * out_color_info.c - show diffrent color when get warnning, note, error ...
 *
 * Copyright (c) 2014-2015 Alan Wang <alan@wrcode.com>
 *
 * This file is released under the Apache Licene 2.0.
 */

#include <stdio.h>
#include <string.h>

#define LINE_SIZE 1024 * 10

// printed color
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define PURPLE "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

// background color
#define BGBLACK "\033[40m"
#define BGRED "\033[41m"
#define BGGREEN "\033[42m"
#define BGYELLOW "\033[43m"
#define BGBLUE "\033[44m"
#define BGPURPLE "\033[45m"
#define BGCYAN "\033[46m"
#define BGWHITE "\033[47m"

// attribute
#define BOLD "\033[1m"
#define COLOR_END "\033[0m"
#define COLOR_NULL ""

#define TRUE 1
#define FALSE 0

/*********************************************************/

#define LOG_CELL_FILE 0x00
#define LOG_CELL_LINE 0x01
#define LOG_CELL_COL 0x02
#define LOG_CELL_ERROR_MARK 0x03
#define LOG_CELL_ERROR_INFO 0x04
#define LOG_CELL_WARNING_MARK 0x05
#define LOG_CELL_WARNING_INFO 0x06
#define LOG_CELL_NOTE_MARK 0x07
#define LOG_CELL_NOTE_INFO 0x08
#define LOG_CELL_BUILD 0x09
#define LOG_CELL_INFO 0x0A
#define LOG_CELL_END 0x0B

struct LogCell_st
{
    const unsigned int type;
    const char*        color_prefix[4];
    const char*        color_suffix;
};
static struct LogCell_st LogCell[LOG_CELL_END] = {
    {LOG_CELL_FILE, {CYAN, BGBLACK, BOLD, COLOR_NULL}, COLOR_END},
    {LOG_CELL_LINE, {PURPLE, COLOR_NULL, COLOR_NULL, COLOR_NULL}, COLOR_END},
    {LOG_CELL_COL, {CYAN, COLOR_NULL, COLOR_NULL, COLOR_NULL}, COLOR_END},
    {LOG_CELL_ERROR_MARK, {RED, BOLD, COLOR_NULL, COLOR_NULL}, COLOR_END},
    {LOG_CELL_ERROR_INFO, {RED, COLOR_NULL, COLOR_NULL, COLOR_NULL}, COLOR_END},
    {LOG_CELL_WARNING_MARK, {YELLOW, BOLD, COLOR_NULL, COLOR_NULL}, COLOR_END},
    {LOG_CELL_WARNING_INFO, {YELLOW, COLOR_NULL, COLOR_NULL, COLOR_NULL}, COLOR_END},
    {LOG_CELL_NOTE_MARK, {CYAN, BOLD, COLOR_NULL, COLOR_NULL}, COLOR_END},
    {LOG_CELL_NOTE_INFO, {CYAN, COLOR_NULL, COLOR_NULL, COLOR_NULL}, COLOR_END},
    {LOG_CELL_BUILD, {WHITE, BOLD, COLOR_NULL, COLOR_NULL}, COLOR_END},
    {LOG_CELL_INFO, {WHITE, COLOR_NULL, COLOR_NULL, COLOR_NULL}, COLOR_NULL}};

#define MSG_INFO_FILE 0x01
#define MSG_INFO_LINE 0x02
#define MSG_INFO_COL 0x03
#define MSG_INFO_MARK 0x04
#define MSG_INFO_INFO 0x05
#define MSG_INFO_COMMENT 0x06

#define SET_STYLE(s0, s1, s2, s3, s4, s5, s6, s7)                                                                      \
    ((s0 << 28) | (s1 << 24) | (s2 << 20) | (s3 << 16) | (s4 << 12) | (s5 << 8) | (s6 << 4) | (s7 << 0))
struct MarkCfg_st
{
    int         start_index; //-1 Any position
    const char* mark_start;
    const char* mark_end;
    const char* endline;
};
struct MsgInfo_st
{
    struct MarkCfg_st  mark;
    struct LogCell_st* p_file;
    struct LogCell_st* p_line;
    struct LogCell_st* p_col;
    struct LogCell_st* p_mark;
    struct LogCell_st* p_info;
    struct LogCell_st* p_comment;
    unsigned int       style;
};

struct SplitStr_st
{
    char* p_file;
    char* p_line;
    char* p_col;
    char* p_mark;
    char* p_info;
    char* p_comment;
    int   file_len;
    int   line_len;
    int   col_len;
    int   mark_len;
    int   info_len;
    int   comment_len;
};

struct MsgInfo_st make_rules[] = {
    {{-1, "CMake Error", NULL, "\r\n"},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_ERROR_MARK],
     &LogCell[LOG_CELL_ERROR_INFO],
     &LogCell[LOG_CELL_ERROR_INFO],
     SET_STYLE(MSG_INFO_MARK, MSG_INFO_COMMENT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)},
    {{-1, "CMake Warning", NULL, "\r\n"},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_WARNING_MARK],
     &LogCell[LOG_CELL_WARNING_INFO],
     &LogCell[LOG_CELL_WARNING_INFO],
     SET_STYLE(MSG_INFO_MARK, MSG_INFO_COMMENT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)},
    {{0, "make[", "]:", NULL},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_NOTE_MARK],
     &LogCell[LOG_CELL_INFO],
     &LogCell[LOG_CELL_INFO],
     SET_STYLE(MSG_INFO_MARK, MSG_INFO_COMMENT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)},
    {{0, "[", "] ", NULL},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_BUILD],
     &LogCell[LOG_CELL_INFO],
     &LogCell[LOG_CELL_INFO],
     SET_STYLE(MSG_INFO_MARK, MSG_INFO_COMMENT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)},
    {{0, "CC ", NULL, NULL},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_BUILD],
     &LogCell[LOG_CELL_NOTE_INFO],
     &LogCell[LOG_CELL_INFO],
     SET_STYLE(MSG_INFO_MARK, MSG_INFO_COMMENT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)},
    {{0, "Depend ", NULL, NULL},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_BUILD],
     &LogCell[LOG_CELL_NOTE_INFO],
     &LogCell[LOG_CELL_INFO],
     SET_STYLE(MSG_INFO_MARK, MSG_INFO_COMMENT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)},
    {{-1, "***", NULL, NULL},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_ERROR_MARK],
     &LogCell[LOG_CELL_ERROR_INFO],
     &LogCell[LOG_CELL_ERROR_INFO],
     SET_STYLE(MSG_INFO_COMMENT, MSG_INFO_MARK, MSG_INFO_INFO, 0x00, 0x00, 0x00, 0x00, 0x00)},
    {{
         -1,
         "warning:",
         NULL,
         NULL,
     },
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_WARNING_MARK],
     &LogCell[LOG_CELL_WARNING_INFO],
     &LogCell[LOG_CELL_NOTE_INFO],
     SET_STYLE(MSG_INFO_FILE, MSG_INFO_LINE, MSG_INFO_COL, MSG_INFO_MARK, MSG_INFO_INFO, 0x00, 0x00, 0x00)},
    {{
         -1,
         "warning #",
         NULL,
         NULL,
     },
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_WARNING_MARK],
     &LogCell[LOG_CELL_WARNING_INFO],
     &LogCell[LOG_CELL_NOTE_INFO],
     SET_STYLE(MSG_INFO_FILE, MSG_INFO_LINE, MSG_INFO_COL, MSG_INFO_MARK, MSG_INFO_INFO, 0x00, 0x00, 0x00)},
    {{-1, "error:", NULL, NULL},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_ERROR_MARK],
     &LogCell[LOG_CELL_ERROR_INFO],
     &LogCell[LOG_CELL_NOTE_INFO],
     SET_STYLE(MSG_INFO_FILE, MSG_INFO_LINE, MSG_INFO_COL, MSG_INFO_MARK, MSG_INFO_INFO, 0x00, 0x00, 0x00)},
    {{-1, "error #", NULL, NULL},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_ERROR_MARK],
     &LogCell[LOG_CELL_ERROR_INFO],
     &LogCell[LOG_CELL_NOTE_INFO],
     SET_STYLE(MSG_INFO_FILE, MSG_INFO_LINE, MSG_INFO_COL, MSG_INFO_MARK, MSG_INFO_INFO, 0x00, 0x00, 0x00)},
    {{-1, "note:", NULL, NULL},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_NOTE_MARK],
     &LogCell[LOG_CELL_NOTE_INFO],
     &LogCell[LOG_CELL_NOTE_INFO],
     SET_STYLE(MSG_INFO_FILE, MSG_INFO_LINE, MSG_INFO_COL, MSG_INFO_MARK, MSG_INFO_INFO, 0x00, 0x00, 0x00)},

    {{-1, "^", NULL, NULL},
     &LogCell[LOG_CELL_FILE],
     &LogCell[LOG_CELL_LINE],
     &LogCell[LOG_CELL_COL],
     &LogCell[LOG_CELL_NOTE_MARK],
     &LogCell[LOG_CELL_NOTE_INFO],
     &LogCell[LOG_CELL_NOTE_INFO],
     SET_STYLE(MSG_INFO_COMMENT, MSG_INFO_MARK, MSG_INFO_INFO, 0x00, 0x00, 0x00, 0x00, 0x00)}};

void color_print_cell(const char* str, struct LogCell_st* p_log_cell, int str_len)
{
    char buf[LINE_SIZE];
    if ((str_len <= 0) || (str == NULL))
    {
        return;
    }
    strncpy(buf, str, str_len);
    buf[str_len] = '\0';
    if (p_log_cell != NULL)
    {
        printf(
            "%s%s%s%s",
            p_log_cell->color_prefix[0],
            p_log_cell->color_prefix[1],
            p_log_cell->color_prefix[2],
            p_log_cell->color_prefix[3]);
        printf("%s", buf);
        printf("%s", p_log_cell->color_suffix);
    }
    else
    {
        printf("%s", buf);
    }
}

struct SplitStr_st spilt_line(char* line, struct MarkCfg_st make_cfg, unsigned int style)
{
    struct SplitStr_st split_info = {NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0};
    // int                style_sign = 0;
    char* p_tmp = NULL;
    switch (style)
    {
    case SET_STYLE(MSG_INFO_FILE, MSG_INFO_LINE, MSG_INFO_COL, MSG_INFO_MARK, MSG_INFO_INFO, 0x00, 0x00, 0x00):
    {
        //../src/scom_mcu10.cpp:79:76: error: expected ';' after top level declarator
        do
        {
            if (NULL == (p_tmp = strstr(line, make_cfg.mark_start)))
                break;
            split_info.p_file   = line;
            split_info.p_mark   = p_tmp;
            split_info.p_info   = p_tmp + strlen(make_cfg.mark_start);
            split_info.mark_len = strlen(make_cfg.mark_start);
            split_info.info_len = strlen(split_info.p_info);
            split_info.file_len = p_tmp - split_info.p_file;
            if (NULL == (p_tmp = strstr(line, ":")))
                break;
            split_info.p_line = p_tmp + 1;
            if (split_info.file_len != 0)
            {
                split_info.file_len = p_tmp - split_info.p_file + 1;
            }
            split_info.line_len = split_info.p_mark - p_tmp;
            if (NULL == (p_tmp = strstr(split_info.p_line, ":")))
                break;
            split_info.p_col = p_tmp + 1;
            if (split_info.line_len != 0)
            {
                split_info.line_len = p_tmp - split_info.p_line + 1;
            }
            split_info.col_len = split_info.p_mark - p_tmp;
            if (NULL == (p_tmp = strstr(split_info.p_col, ":")))
                break;
            split_info.col_len = p_tmp - split_info.p_col + 1;
        } while (0);
        break;
    }
    case SET_STYLE(MSG_INFO_MARK, MSG_INFO_COMMENT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00):
    {
        do
        {
            if (NULL == (p_tmp = strstr(line, make_cfg.mark_start)))
                break;
            split_info.p_mark      = p_tmp;
            split_info.mark_len    = strlen(make_cfg.mark_start);
            split_info.p_comment   = p_tmp + strlen(make_cfg.mark_start);
            split_info.comment_len = strlen(split_info.p_comment);
            if (make_cfg.mark_end == NULL)
                break;
            if (NULL == (p_tmp = strstr(line, make_cfg.mark_end)))
                break;
            split_info.p_comment   = p_tmp + 1;
            split_info.mark_len    = p_tmp - split_info.p_mark + 1;
            split_info.comment_len = strlen(split_info.p_comment);
        } while (0);
        break;
    }
    case SET_STYLE(MSG_INFO_COMMENT, MSG_INFO_MARK, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00):
    {
        do
        {
            if (NULL == (p_tmp = strstr(line, make_cfg.mark_start)))
                break;
            split_info.p_comment   = line;
            split_info.comment_len = p_tmp - line;
            split_info.p_mark      = p_tmp;
            split_info.mark_len    = strlen(make_cfg.mark_start);
        } while (0);
        break;
    }
    case SET_STYLE(MSG_INFO_MARK, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00):
    {
        do
        {
            if (NULL == (p_tmp = strstr(line, make_cfg.mark_start)))
                break;
            split_info.p_mark   = p_tmp;
            split_info.mark_len = strlen(make_cfg.mark_start);
            if (make_cfg.mark_end == NULL)
                break;
            if (NULL == (p_tmp = strstr(line, make_cfg.mark_end)))
                break;
            split_info.mark_len = p_tmp - split_info.p_mark + 1;
        } while (0);
        break;
    }
    case SET_STYLE(MSG_INFO_COMMENT, MSG_INFO_MARK, MSG_INFO_INFO, 0x00, 0x00, 0x00, 0x00, 0x00):
    {
        do
        {
            if (NULL == (p_tmp = strstr(line, make_cfg.mark_start)))
                break;
            split_info.p_comment   = line;
            split_info.comment_len = p_tmp - line;
            split_info.p_mark      = p_tmp;
            split_info.mark_len    = strlen(make_cfg.mark_start);
            split_info.p_info      = p_tmp + strlen(make_cfg.mark_start);
            split_info.info_len    = strlen(split_info.p_info);
        } while (0);
        break;
    }
    default:
        split_info.p_comment   = line;
        split_info.comment_len = strlen(line);
        break;
    }
    return split_info;
}

void color_line_out(struct SplitStr_st spilt_line, unsigned int style, struct MsgInfo_st msg_info)
{
    int i          = 0;
    int sytle_sign = 0;
    for (i = 0; i < 8; i++)
    {
        sytle_sign = (style >> 28) & 0x0F;
        style      = style << 4;
        // printf("sytle_sign = %d\n", sytle_sign);
        switch (sytle_sign)
        {
        case MSG_INFO_FILE:
            color_print_cell(spilt_line.p_file, msg_info.p_file, spilt_line.file_len);
            break;
        case MSG_INFO_LINE:
            color_print_cell(spilt_line.p_line, msg_info.p_line, spilt_line.line_len);
            break;
        case MSG_INFO_COL:
            color_print_cell(spilt_line.p_col, msg_info.p_col, spilt_line.col_len);
            break;
        case MSG_INFO_MARK:
            color_print_cell(spilt_line.p_mark, msg_info.p_mark, spilt_line.mark_len);
            break;
        case MSG_INFO_INFO:
            color_print_cell(spilt_line.p_info, msg_info.p_info, spilt_line.info_len);
            break;
        case MSG_INFO_COMMENT:
            color_print_cell(spilt_line.p_comment, msg_info.p_comment, spilt_line.comment_len);
            break;
        }
    }
}

void color_line(char* line)
{
    int                mark_rule_total = sizeof(make_rules) / sizeof(struct MsgInfo_st);
    int                i               = 0;
    int                match_flag      = FALSE;
    struct SplitStr_st split_info;

    static struct MsgInfo_st* p_end_msg_info = NULL;

    if (p_end_msg_info != NULL)
    {
        color_print_cell(line, p_end_msg_info->p_info, strlen(line));
        if (strcmp(line, p_end_msg_info->mark.endline) == 0)
        {
            p_end_msg_info = NULL;
        }
        return;
    }
    for (i = 0; i < mark_rule_total; i++)
    {
        if (((make_rules[i].mark.start_index == -1) && (strstr(line, make_rules[i].mark.mark_start) != NULL)) ||
            strncmp(&line[0], make_rules[i].mark.mark_start, strlen(make_rules[i].mark.mark_start)) == 0)
        {
            // printf("++++++++++++++++++++++++++++++++++++++++\n");
            // split the line
            split_info = spilt_line(line, make_rules[i].mark, make_rules[i].style);
            // print the color line
            color_line_out(split_info, make_rules[i].style, make_rules[i]);
            match_flag = TRUE;
            if (make_rules[i].mark.endline != NULL)
            {
                p_end_msg_info = &make_rules[i];
            }
            // printf("=========================================\n");
            break;
        }
    }
    if (match_flag == FALSE)
    {
        printf("%s", line);
    }
}

/*------------------ main func ------------------*/
int main(void)
{
    char line[LINE_SIZE];
    // int   i;
    // int   len;
    // char* p;

    while (1)
    {
        if (0 != feof(stdin) || NULL == fgets(line, LINE_SIZE, stdin))
        {
            break;
        }
        color_line(line);
    }

    return 0;
}
