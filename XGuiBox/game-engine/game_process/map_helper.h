#pragma once
#include <vector>
#include "structs/city_struct.h"
#include "../resources/window_profiling/window.h"

class map_helper
{
public:
    std::vector<city> ( &create_cities() ) [19]
	{
        static bool init = false;
        static std::vector< city > city_data[19];

        if (!init)
        {
            city_data[g_window.countries_name::USA] =
            {
                { "Barrow", ImVec2(42, 201) },
                { "Nome", ImVec2(15, 261) },
                { "Fairbanks", ImVec2(72, 257) },
                { "Bethel", ImVec2(24, 291) },
                { "Anchorage", ImVec2(65, 286) },
                { "Whitehorse", ImVec2(113, 286) },
                { "Juneau", ImVec2(119, 312) },
                { "Yellowknife", ImVec2(191.031555, 277.857788) },
                { "Churchill", ImVec2(259.339233, 305.550110) },
                { "Nain", ImVec2(369.185455, 312.627045) },
                { "Vancouver", ImVec2(148.569977, 356.319336) },
                { "Calgary", ImVec2(181.185364, 349.550110) },
                { "Edmonton", ImVec2(195.031525, 329.550110) },
                { "Saskatoon", ImVec2(219.954605, 341.242401) },
                { "Regina", ImVec2(223.193100, 353.199554) },
                { "Winnipeg", ImVec2(251.339233, 355.703949) },//320.866608, 377.498627
                { "Ottawa", ImVec2(320.866608, 377.498627) },
                { "Quebec", ImVec2(332.558929, 367.960144) },
                { "Toronto", ImVec2(306.712738, 388.883240) },
                { "Charlottetown", ImVec2(360.251251, 375.344757) },
                { "Seattle", ImVec2(156.335510, 368.888214) },
                { "Helena", ImVec2(186.049805, 373.173950) },
                { "San Jose", ImVec2(162.621231, 417.459686) },
                { "Los Angeles", ImVec2(174.049805, 429.745392) },
                { "Las Vegas", ImVec2(183.764084, 419.745392) },
                { "Bismarck", ImVec2(238.621246, 374.316803) },
                { "Oklahoma City", ImVec2(252.049820, 420.888245) },
                { "Dallas", ImVec2(257.520111, 436.963287) },
                { "Houston", ImVec2(258.696564, 448.727997) },
                { "New York", ImVec2(324.587738, 397.249176) },
                { "Washington D.C", ImVec2(319.921051, 408.915833) },
                { "Atlanta", ImVec2(292.587708, 432.249176) },
                { "Mexico City", ImVec2(241.254333, 483.915894) },
                { "Monterrey", ImVec2(231.254318, 460.915863) },
                { "Merida", ImVec2(274.751282, 482.769775) },
                { "Santa Fe", ImVec2(215.197632, 413.709595) },
            };

            city_data[g_window.countries_name::LatinUSA] =
            {
                { "Managua", ImVec2(22.737667, 54.650902) },
                { "Tegucigalpa", ImVec2(18.407806, 46.787113) },

            };

            city_data[g_window.countries_name::SouthernUSA] =
            {
                { "Caracas", ImVec2(150.090164, 8.688351) },
                { "Bogota", ImVec2(123.423492, 30.288361) },
                { "Trujillo", ImVec2(105.290154, 69.755043) },
                { "Lima", ImVec2(115.423492, 88.688385) },
                { "Antofagasta", ImVec2(135.690170, 127.088402) },
                { "Puerto Montt", ImVec2(126.090126, 199.678741) },
                { "Sucre", ImVec2(152.756805, 107.945358) },
                { "Rio Gallegos", ImVec2(138.356796, 248.478760) },
                { "Santiago", ImVec2(132.756790, 168.478714) },
                { "Buenos Aires", ImVec2(176.490143, 173.812057) },
                { "Asuncion", ImVec2(180.490158, 131.678711) },
                { "Porto Alegre", ImVec2(200.490158, 149.812469) },
                { "Brasillia", ImVec2(210.890167, 101.012444) },
                { "Salvador", ImVec2(242.623520, 88.745773) },
                { "Paramaribo", ImVec2(187.423492, 25.279079) },
                { "Manaus", ImVec2(176.756821, 60.212429) },
                { "Caxias", ImVec2(228.756836, 63.412430) },
                { "Campo Grande", ImVec2(187.690140, 114.345711) },
                { "Ji-Parana", ImVec2(165.556808, 81.279037) },

            };

            city_data[g_window.countries_name::EC] =
            {
                { "Reykjavik", ImVec2(180.253571, 278.350311) },
                { "Dublin", ImVec2(228.253098, 353.550262) },
                { "London", ImVec2(251.719772, 362.083588) },
                { "Glasgow", ImVec2(237.853104, 337.550262) },
                { "Paris", ImVec2(261.586761, 375.950256) },
                { "Hague", ImVec2(269.586761, 359.950256) },
                { "Berlin", ImVec2(295.453461, 359.950256) },
                { "Hamburg", ImVec2(287.986786, 349.816925) },
                { "Nuremberg", ImVec2(132.756790, 168.478714) },
                { "Madrid", ImVec2(240.520081, 419.950287) },
                { "Lisbon", ImVec2(221.853409, 425.016968) },
                { "Rome", ImVec2(294.653442, 409.550293) },
                { "Marseille", ImVec2(268.855408, 403.945953) },
                { "Bern", ImVec2(281.805145, 387.941315) },
                { "Palermo", ImVec2(298.578674, 429.731445) },
            };

            city_data[g_window.countries_name::Russia] =
            {
                { "Moscow", ImVec2(75.428047, 295.3076781) },
                { "Kazan", ImVec2(113.246201, 295.307678) },
                { "Samara", ImVec2(113.110039, 305.011139) },
                { "Volgograd", ImVec2(84.882584, 328.580383) },
                { "Ekaterinburg", ImVec2(145.190887, 280.201813) },
                { "Saint-Petersburg", ImVec2(37.580566, 263.599365) },
                { "Voronezh", ImVec2(63.462933, 314.893494) },
                { "Omsk", ImVec2(194.756790, 294.422913) },
                { "Novosibirsk", ImVec2(221.109756, 297.011139) },
                { "Krasnoyarsk", ImVec2(255.227402, 283.364075) },
                { "Stavropol", ImVec2(78.802628, 350.706665) },
                { "Yaroslavl", ImVec2(78.555107, 277.425079) },
                { "Irkutsk", ImVec2(294.303864, 311.079010) },
                { "Yakutsk", ImVec2(373.672241, 263.079041) },
                { "Chelyabinsk", ImVec2(150.591156, 293.271027) },
                { "Chita", ImVec2(324.235443, 304.329803) }, //55.768299, 283.754547
                { "Tver", ImVec2(55.768299, 283.754547) },
                { "Vologda", ImVec2(80.625450, 262.040253) },
                { "Kirov", ImVec2(116.339752, 265.754547) },
                { "Petrozavodsk", ImVec2(51.196869, 245.754532) },
                { "Nizhny Tagil", ImVec2(145.768341, 265.183105) }, //95.010284, 311.695160
                { "Saratov", ImVec2(95.010284, 311.695160) },
                { "Vorkuta", ImVec2(151.801376, 201.475372) },
                { "Vladivostok", ImVec2(393.651520, 354.551025) },
                { "Grozniy", ImVec2(89.959358, 356.455750) },

            };

            city_data[g_window.countries_name::Northeurope] =
            {
                { "Helsinki", ImVec2(117.252304, 230.247894) },
                { "Tallinn", ImVec2(117.252304, 236.247894) },
                { "Vaasa", ImVec2(106.752304, 209.747894) },
                { "Oslo", ImVec2(69.252289, 230.247894) },
                { "Stockholm", ImVec2(93.752289, 236.747910) },
                { "Sundsvall", ImVec2(91.164513, 214.858780) },
                { "Lulea", ImVec2(104.764526, 191.458786) },
            };

            city_data[g_window.countries_name::EastEC] =
            {
                { "Warsaw", ImVec2(30.130413, 18.204439) },
                { "Krakow", ImVec2(25.178030, 27.156826) },
                { "Bratislava", ImVec2(21.939932, 36.299686) },
                { "Budapest", ImVec2(23.273266, 44.109219) },
                { "Sofia", ImVec2(40.416134, 63.918755) },
                { "Athens", ImVec2(44.797089, 86.204483) },
                { "Bucharest", ImVec2(48.987568, 54.775890) },
            };

            city_data[g_window.countries_name::east_europe] =
            {
                { "Minsk", ImVec2(21.125013, 15.978427) },
                { "Kyiv", ImVec2(29.505972, 32.168915) },
                { "Kharkiv", ImVec2(47.029793, 36.359394) },
                { "Zhytomyr", ImVec2(16.721037, 33.881676) },
                { "Chisinau", ImVec2(23.029776, 50.073689) },
            };

            city_data[g_window.countries_name::Turk] =
            {
                { "Ankara", ImVec2(28.300053, 12.651188) },
                { "Damascus", ImVec2(40.300060, 36.270248) },
                { "Baghdad", ImVec2(61.823879, 38.555965) },
                { "Kirkuk", ImVec2(61.633404, 27.889292) },
                { "Samsun", ImVec2(37.252438, 4.270231) },
                { "Konya", ImVec2(22.966717, 18.746429) },
            };

            city_data[g_window.countries_name::North_WellWellWell] =
            {
                { "Riyadh", ImVec2(223.580673, 55.771702) },
                { "Aden", ImVec2(215.135040, 91.993950) },
                { "Cairo", ImVec2(165.801666, 32.660572) },
                { "Khartoum", ImVec2(169.357224, 78.438385) },
                { "N'Djamena", ImVec2(114.377556, 94.006775) },
                { "Zinder", ImVec2(91.789337, 90.242073) },
                { "Niamey", ImVec2(69.436409, 91.418541) },
                { "Abuja", ImVec2(86.994431, 107.771454) },
                { "Yamoussoukro", ImVec2(41.244473, 113.521454) },
                { "Dakar", ImVec2(7.244502, 65.771515) },
                { "Rabat", ImVec2(35.994480, 20.521545) },
                { "Bamako", ImVec2(35.994480, 92.771492) },
                { "Algiers", ImVec2(70.994453, 7.521555) },
                { "Tripoli", ImVec2(105.744431, 21.771545) },
            };

            city_data[g_window.countries_name::MidWellWellWell] =
            {
                { "Malabo", ImVec2(4.715996, 38.075298) },
                { "Bangui", ImVec2(32.480724, 34.781178) },
                { "Libreville", ImVec2(6.363057, 53.369427) },
                { "Juba", ImVec2(81.186646, 29.839996) },
                { "Addis Ababa", ImVec2(105.657249, 22.310579) },
            };

            city_data[g_window.countries_name::South_WellWellWell] =
            {
                { "Kinshasa", ImVec2(23.277018, 24.023441) },
                { "Bumba", ImVec2(36.218204, 17.199907) },
                { "Kananga", ImVec2(37.865265, 39.082279) },
                { "Kampala", ImVec2(72.453529, 18.141085) },
                { "Nairobi", ImVec2(89.159424, 23.552853) },
                { "Dodoma", ImVec2(80.191437, 41.762150) },
                { "Lusaka", ImVec2(55.691429, 72.012161) },
                { "Harare", ImVec2(64.941437, 82.012161) },
                { "Maputo", ImVec2(78.191437, 102.512177) },
                { "Bloemfontein", ImVec2(46.191425, 121.012184) },
                { "Upington", ImVec2(33.691422, 118.512177) },
                { "Cape Town", ImVec2(23.941416, 142.762192) },
                { "Windhoek", ImVec2(18.191414, 97.012169) },
            };

            city_data[g_window.countries_name::Indostan] =
            {
                { "Muscat", ImVec2(49.238419, 65.549530) },
                { "Tehran", ImVec2(27.764719, 16.917917) },
                { "Yazd", ImVec2(43.343678, 38.812668) },
                { "Bamian", ImVec2(81.448250, 22.602131) },
                { "Kandahar", ImVec2(71.974556, 35.233719) },
                { "Islamabad", ImVec2(101.027206, 27.444239) },
                { "Buhawalpur", ImVec2(90.290359, 43.654778) },
                { "New Delhi", ImVec2(116.185112, 50.391624) },
                { "Chandigarh", ImVec2(112.185112, 40.286354) },
                { "Rajkot", ImVec2(91.553520, 71.865326) },
                { "Solapur", ImVec2(108.816689, 85.760071) },
                { "Salem", ImVec2(116.185112, 105.128502) },
                { "Madurai", ImVec2(117.869324, 110.391663) },
                { "Dhaka", ImVec2(159.512756, 65.459679) },
                { "Naypyidaw", ImVec2(178.979431, 80.926346) },
                { "Bhilai", ImVec2(125.359772, 73.526421) },
                { "Bhubaneswar", ImVec2(142.833466, 78.368530) },
            };

            city_data[g_window.countries_name::Zakavkazie] =
            {
                { "Yerevan", ImVec2(17.043537, 14.748335) }
            };

            city_data[g_window.countries_name::Churki] =
            {
                { "Astana", ImVec2(86.299988, 22.282675) },
                { "Atyrau", ImVec2(20.888172, 45.576813) },
                { "Kyzylorda", ImVec2(58.064671, 58.517998) },
                { "Bishkek", ImVec2(96.417641, 67.929771) },
                { "Tashkent", ImVec2(78.064690, 75.459190) },
                { "Samarkand", ImVec2(66.535263, 80.870956) },
                { "Ashgabat", ImVec2(39.005833, 84.635666) },
                { "Nukus", ImVec2(46.702759, 68.027840) },
                { "Dushanbe", ImVec2(81.996902, 85.439621) },
            };

            city_data[g_window.countries_name::China] =
            {
                { "Ulaanbaatar", ImVec2(118.749847, 36.044868) },
                { "Erdenet", ImVec2(103.321190, 22.901987) },
                { "Beijing", ImVec2(146.900284, 69.679459) },
                { "Zibo", ImVec2(153.566956, 80.568359) },
                { "Shanghai", ImVec2(161.344727, 101.012810) },
                { "Hong Kong", ImVec2(139.541885, 135.640305) },
                { "Kunming", ImVec2(97.941864, 128.240295) },
                { "Wuhan", ImVec2(137.541885, 103.840279) },
                { "Chengdu", ImVec2(99.541862, 104.240280) },
                { "Guilin", ImVec2(121.646561, 120.686592) },
                { "Xi'an", ImVec2(116.313225, 93.067520) },
                { "Taiyuan", ImVec2(125.456085, 76.877037) },
                { "Baotou", ImVec2(121.075134, 66.019890) },
                { "Golmud", ImVec2(84.884636, 87.924660) },
                { "Hotan", ImVec2(27.551264, 81.638947) },
                { "Xining", ImVec2(88.884636, 80.115135) },
                { "Korla", ImVec2(47.668518, 59.457573) },
                { "Fuzhou", ImVec2(156.193161, 122.709167) },
            };

            city_data[g_window.countries_name::Indo_China] =
            {
                { "Hanoi", ImVec2(28.221739, 10.504876) },
                { "Vientiane", ImVec2(19.650307, 21.171547) },
                { "Bangkok", ImVec2(12.602684, 35.838223) },
                { "Phnom Penh", ImVec2(26.126499, 41.933460) },
                { "Pleiku", ImVec2(38.507458, 34.695515) }
            };

            city_data[g_window.countries_name::Samurai] =
            {
                { "Kuala Lumpur", ImVec2(24.590548, 166.036514) },
                { "Medan", ImVec2(18.695816, 173.404922) },
                { "Jakarta", ImVec2(41.643169, 199.089127) },
                { "Merauke", ImVec2(153.432571, 202.247009) },
                { "Jayapura", ImVec2(156.317337, 186.457550) },
                { "Port Moresby", ImVec2(180.379623, 206.668060) },
                { "Lae", ImVec2(177.642776, 199.089127) },
                { "Hanoi", ImVec2(28.221739, 10.504876) },
                { "Seoul", ImVec2(112.457100, 39.075222) },
                { "Tokyo", ImVec2(153.409515, 44.789509) },
                { "Hiroshima", ImVec2(129.219025, 48.979988) },
                { "Miyazaki", ImVec2(123.885681, 60.979996) },
                { "Asahikawa", ImVec2(163.884384, 8.789462) },
                { "Sendai", ImVec2(158.641373, 32.061432) },
                { "Bandar Seri Begawan", ImVec2(68.164940, 161.013672) },
                { "Bontang", ImVec2(75.783989, 174.918350) },
                { "Banjarmasin", ImVec2(64.736366, 187.680252) },
                { "Pontianak", ImVec2(48.736355, 174.346909) },
                { "General Santos", ImVec2(103.451828, 152.861115) },
                { "Manila", ImVec2(89.940163, 126.277924) },
                { "Osaka", ImVec2(141.703568, 47.655811) },
                { "Singapore", ImVec2(32.180634, 172.988953) },
            };

            city_data[g_window.countries_name::Austrilia] =
            {
                { "Canberra", ImVec2(124.383881, 98.622498) },
                { "Melbourne", ImVec2(105.494980, 106.844727) },
                { "Perth", ImVec2(10.383797, 79.289146) },
                { "Darwin", ImVec2(62.828281, 10.844653) },
                { "Townsville", ImVec2(114.828316, 31.066891) },
                { "Brisbane", ImVec2(138.161667, 65.066917) },
                { "Hanoi", ImVec2(28.221739, 10.504876) },
                { "Alice Springs", ImVec2(73.050507, 57.955799) },
                { "Broken Hill", ImVec2(99.272850, 83.511391) },
                { "Mount Isa", ImVec2(92.828400, 34.844696) },
                { "Rockhampton", ImVec2(128.383972, 45.955814) },
                { "Griffth", ImVec2(111.272858, 89.733620) },
                { "Auckland", ImVec2(213.210007, 99.786263) },
                { "Christchurch", ImVec2(199.432220, 132.452957) }
            };
        }
        return city_data;

	}
	
};
inline map_helper g_map_helper;