/*
******************************************************************************
* Locke's   __ -based on merc v2.2-____        NIM Server Software           *
* ___ ___  (__)__    __ __   __ ___| G| v4.0   Version 4.0 GOLD EDITION      *
* |  /   \  __|  \__/  |  | |  |     O|        documentation release         *
* |       ||  |        |  \_|  | ()  L|        Hallow's Eve 1999             *
* |    |  ||  |  |__|  |       |     D|                                      *
* |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
*   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
******************************************************************************
 */


/*
 * Types of components.
 */
#define C_LIQUID              0
#define C_GASEOUS             1
#define C_SOLID               2
#define C_GELATIN             0
#define C_POWDER              4
#define C_ROD                 5
#define C_WIRE                6
#define C_GEM                 7
#define C_MEMBRANE            8



/*
 * Spell functions.
 * Defined in magic.c.
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(      spell_call_tidal_wave   );
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(      spell_channeling        );
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(      spell_light             );
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(	spell_detect_evil	);
DECLARE_SPELL_FUN(	spell_detect_hidden	);
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(      spell_general_purpose   );
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(	spell_high_explosive	);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_prop	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_protection	);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(      spell_water_breathing   );
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);
 */


/*
 * Spell components.


   -- Metals
       Brass cylinder
       Bronze, disk
    -  Copper
        square
        wire
    -  Gold
        bell
        caltrops
        chunk
        dust
        needles
        powder
        tissue
        wire
    -  Iron
        chunk
        filings
        powder
        rod
    -  Lead
        ball
        chunk
       Mercury
    -  Mica
        chipped
        ground
    -  Platinum
        chunk
        coin
        die
        mirror
        tissue
        wire
    -  Silver
        pin
        powder
        wire
       Tin square
       Zinc square       

   -- Liquids
       Acid
       Alcohol
       Attar of roses
       Hickory oil
       Ink (black)
       Molasses
    -  Oil
        flammable
        nonflammable
        spiced
        sweet
       Rainwater
       Vinegar
       Water, distilled

   -- Minerals
       Agate
       Alkaline salt
       Alum
    -  Amber
        block
        powder
        rod
       Bitumen
       Blue vitriol
    -  Borax
        paste
        stone
       Bromine salt
    -  Carbon
        amorphous
        graphite
       Chalk
       Charcoal
    -  Clay
        fired spheres
        raw
    -  Coal (anthracite)
        chunk
        dust
    -  Crystal
        bead/marble
        rod
        square
        sphere
       Diamond
       Emerald
       Feldspar chunk
       Flint
    -  Glass
        bead
        cone
        rod
        sheet
        sphere
    -  Granite
        block
        dust
       Gypsum
       Iron pyrite
       Lime powder
       Litharge
    -  Marble (chunk)
        black
        purbeck
    -  Mineral
        spheres
        prisms
       Mortar (powder)
       Nitre
       Nitre, salt
    -  Obsidian
        chunk
        cylinder
        sphere
       Phosphorous
       Pitch
    -  Quartz
        clear rough
        other rough
        powder
       Ruby
    -  Salt
        block
        granular
       Sapphire
       Sand, fine
       Soot
    -  Stalactite
        shards
        tiny
       Sulfer
       Talc
    -  Turquoise
        polished
        unpolished
       Volcanic ash

   -- Animal Products
       Bat fur
       Bat guano
       Beeswax
       Bee wing
       Bladder (balloon)
    -  Bone (bovine)
        powdered
        chunks
       Blood (bovine)
    -  Cocoon
        butterfly
        moth
        wasp
       Coral chunk
       Eelskin
    -  Eggshell
        crushed (powdered)
        shards        
        whole (empty)
       Fat (bovine)
    -  Feather
        chicken tail  
        eagle
        hawk
        owl
        raven
        white
        wing
        vulture tail       
       Firefly (dead)
       Fleece
       Fish scale
       Frog leg
    -  Glowworm
        tails
        whole (dead)
    -  Hair
        ape
        bear
        camel
        draft horse
        child's
        ox
        war horse
       Honeycomb
    -  Insect legs
        centipede
        grasshopper (hind)
        millipede
        spider
    -  Leather
        strips
        thong
       Lime
       Lye
       Milk fat (solid)
    -  Mouse
        whiskers
        whole (dead)
       Musk
    -  Ox hoof 
        whole
        powdered
       Pork rind
       Python bile
    -  Rat
        tail
        whisker
        whole (dead)
    -  Seashell
        shards
        whole
       Skunk hair
       Snake skin
       Silkworm eggs
       Snake scale
       Sponge
       Tallow
       Tortoise shell
       Vellum
       Wax (white)
       Wool

   -- Plant Products
       Acorns
    -  Amaryllis
        blossom (dry)
        bulb, common
        bulb, hypoxis
        stalks (dry)
       Apple blossom petal
       Arsenic
       Asafoetida
       Aster seed
    -  Bark
        apple
        ash
        beech
        cherry
        elm
        maple
        oak
        pine
        spruce
        sumac
        walnut
       Burrs
       Camphor resin
       Carrot (dried)
    -  Cassia
        bark
        buds
       Castor beans
       Citric acid
       Cork
       Corn extract (powdered)
       Cotton mesh
       Felt, white
       Fern frond
       Flower
       Foxfire fungus
       Galbanum
    -  Garlic
        clove
        bud
       Ginger
       Ginseng
       Gum arabic
    -  Holly
        berries
        sprig
       Incense (stick)
    -  Leaves
        ash
        oak
        poison ivy
        poison oak
        poison sumac
        raspberry
        rose
        rhubarb
        shamrock
        skunk cabbage
        stinging nettle
        sumac
        yew
       Leeks (ground)
       Legume seeds
       Linen cloth (gold wefted)
    -  Lotus
        flower
        root
       Marigolds (dry crushed)
       Mayz
       Moonseed
    -  Moss
        green
        phosphorescent
    -  Mushscene
        spores
        whole
       Nut shells
       Pillow ticking
    -  Pine
        bark
        cones
        needles
        sprig
        tar
       Peach pits
       Realgar
       Reed stalks
       Resin
    -  Rose
        leaf
        petals
        thorn
    -  Rhubarb
        leaf
        root
       Seaweed (dry)
       Shallot bulb
       Soda ash
       Stargrass
       Tacamahac
       Thistledown
       Thorns
       Tree sap
       Vermilion
       Willow bark
       Wolfsbane
       Wynchwood
       Yam
       Zedoary
       
   -- Aromatics and Spices
       Aloeswood chips
       Ambergris
       Benzoin
       Betony
       Calmus
       Civet
       Emeli
       Frankincense
       Myrrh
       Onycha
       Orchid (dried)
       Patchouli (dry)
       Putchuk
       Rosewater
    -  Sandalwood oil
        red
        white
       Storax
 */


