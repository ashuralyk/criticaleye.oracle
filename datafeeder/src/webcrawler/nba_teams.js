const nba_teams = [
    { teamId: 0, teamName: '马刺' },
    { teamId: 1, teamName: '灰熊' },
    { teamId: 2, teamName: '独行侠' },
    { teamId: 3, teamName: '火箭' },
    { teamId: 4, teamName: '鹈鹕' },
    { teamId: 5, teamName: '森林狼' },
    { teamId: 6, teamName: '掘金' },
    { teamId: 7, teamName: '爵士' },
    { teamId: 8, teamName: '开拓者' },
    { teamId: 9, teamName: '雷霆' },
    { teamId: 10, teamName: '国王' },
    { teamId: 11, teamName: '太阳' },
    { teamId: 12, teamName: '湖人' },
    { teamId: 13, teamName: '快船' },
    { teamId: 14, teamName: '勇士' },
    { teamId: 15, teamName: '热火' },
    { teamId: 16, teamName: '魔术' },
    { teamId: 17, teamName: '老鹰' },
    { teamId: 18, teamName: '奇才' },
    { teamId: 19, teamName: '黄蜂' },
    { teamId: 20, teamName: '活塞' },
    { teamId: 21, teamName: '步行者' },
    { teamId: 22, teamName: '骑士' },
    { teamId: 23, teamName: '公牛' },
    { teamId: 24, teamName: '雄鹿' },
    { teamId: 25, teamName: '凯尔特人' },
    { teamId: 26, teamName: '76人' },
    { teamId: 27, teamName: '尼克斯' },
    { teamId: 28, teamName: '篮网' },
    { teamId: 29, teamName: '猛龙' },
  ];
  
  module.exports = {
    nba_teams,
    teamId,
    teamName
  };
  
function teamId(name) {
  const team = nba_teams.find(t => t.teamName === name);
  return team ? team.teamId : undefined;
}
  
function teamName(id) {
  const team = nba_teams.find(t => t.teamId == id);
  return team ? team.teamName : undefined;
}
