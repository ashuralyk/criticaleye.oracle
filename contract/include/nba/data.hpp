
#ifndef _INCLUDE_DATA_NBA_
#define _INCLUDE_DATA_NBA_

#include <map>
#include <string>
#include <tuple>

#define nba_contract "oracleosxnba"

using namespace std;

namespace nba
{

inline tuple<string, string, string, string> get_team_by_id( uint8_t team_id )
{
    struct NBA
    {
        string short_name_en;
        string short_name_chs;
        string full_name_en;
        string full_name_chs;
    };

    const static map<uint8_t, NBA> id_to_team = {
        { 0,  { "SAS", "马刺",    "San Antonio Spurs",      "圣安东尼奥马刺队" } },
        { 1,  { "MEM", "灰熊",    "Memphis Grizzlies",      "孟菲斯灰熊队" } },
        { 2,  { "DAL", "独行侠",  "Dallas Mavericks",       "达拉斯独行侠队" } },
        { 3,  { "HOU", "火箭",    "Houston Rockets",        "休斯顿火箭队" } },
        { 4,  { "NOP", "鹈鹕",    "New Orleans Pelicans",   "新奥尔良鹈鹕队" } },
        { 5,  { "MIN", "森林狼",  "Minnesota Timberwolves", "明尼苏达森林狼" } },
        { 6,  { "DEN", "掘金",    "Denver Nuggets",         "丹佛掘金队" } },
        { 7,  { "UTA", "爵士",    "Utah Jazz",              "犹他爵士队" } },
        { 8,  { "POR", "开拓者",  "Portland Trail Blazers", "波特兰开拓者队" } },
        { 9,  { "OKC", "雷霆",    "Oklahoma City Thunder",  "俄克拉荷马雷霆队" } },
        { 10, { "SAC", "国王",    "Sacramento Kings",       "萨克门托国王队" } },
        { 11, { "PHX", "太阳",    "Phoenix Suns",           "菲尼克斯太阳队" } },
        { 12, { "LAL", "湖人",    "Los Angeles Lakers",     "洛杉矶湖人队" } },
        { 13, { "LAC", "快船",    "Los Angeles Clippers",   "洛杉矶快船队" } },
        { 14, { "GSW", "勇士",    "Golden State Warriors",  "金州勇士队" } },
        { 15, { "MIA", "热火",    "Miami Heat",             "迈阿密热火队" } },
        { 16, { "ORL", "魔术",    "Orlando Magic",          "奥兰多魔术队" } },
        { 17, { "ATL", "老鹰",    "Atlanta Hawks",          "亚特兰大老鹰队" } },
        { 18, { "WAS", "奇才",    "Washington Wizards",     "华盛顿奇才" } },
        { 19, { "CHA", "黄蜂",    "Charlotte Hornets",      "夏洛特黄蜂" } },
        { 20, { "DET", "活塞",    "Detroit Pistons",        "底特律活塞队" } },
        { 21, { "IND", "步行者",  "Indiana Pacers",         "印第安纳步行者队" } },
        { 22, { "CLE", "骑士",    "Cleveland Cavaliers",    "克里弗兰骑士队" } },
        { 23, { "CHI", "公牛",    "Chicago Bulls",          "芝加哥公牛队" } },
        { 24, { "MIL", "雄鹿",    "Milwaukee Bucks",        "密尔沃基雄鹿队" } },
        { 25, { "BOS", "凯尔特人", "Boston Celtics",         "波士顿凯尔特人队" } },
        { 26, { "PHI", "76人",    "Philadelphia 76ers",     "费城76人队" } },
        { 27, { "NYK", "尼克斯",   "New York Knicks",       "纽约尼克斯队" } },
        { 28, { "BKN", "篮网",    "New Jersey Nets;",       "新泽西篮网队" } },
        { 29, { "TOR", "猛龙",    "Toronto Raptors",        "多伦多猛龙队" } }
    };

    if ( auto i = id_to_team.find(team_id); i != id_to_team.end() )
    {
        const auto &nba = (*i).second;
        return { nba.short_name_en, nba.short_name_chs, nba.full_name_en, nba.full_name_chs };
    }
    else
    {
        return { "", "", "", "" };
    }
}

struct data
{
    string   game_id;
    int32_t  game_start_time;
    uint8_t  home_team_id;
    uint8_t  home_team_score;
    uint8_t  away_team_id;
    uint8_t  away_team_score;
    uint8_t  status;
};

}

#endif
