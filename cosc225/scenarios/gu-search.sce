* GUSEARCH VERBOSE SEARCH ON
* GUSEARCH VERBOSE CHORD ON

TIME 120000
0 GUSEARCH CHORD JOIN 0
TIME 10000
1 GUSEARCH CHORD JOIN 0
TIME 10000
2 GUSEARCH CHORD JOIN 1
TIME 10000
0 GUSEARCH PUBLISH ./cosc225/keys/metadata0.keys
TIME 10000
1 GUSEARCH PUBLISH ./cosc225/keys/metadata1.keys
TIME 10000
0 GUSEARCH SEARCH 0 T1 T2
TIME 10000
1 GUSEARCH SEARCH 1 T2
TIME 10000
2 GUSEARCH SEARCH 2 T10
TIME 10000
1 GUSEARCH CHORD LEAVE
TIME 10000
2 GUSEARCH SEARCH 2 lady gaga
TIME 10000
0 GUSEARCH CHORD RINGSTATE
