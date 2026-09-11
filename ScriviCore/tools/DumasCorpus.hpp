#pragma once
// DumasCorpus.hpp — SP-128 / T-0501 ([I-0195]) — the shared test-world corpus.
//
// ⚠️ ONE CORPUS, TWO BUILDERS. The Qt fixture
// (platforms/linux/tests/dumas_world_fixture.cpp) and the Qt-free native tool
// (ScriviCore/tools/scrivi_make_test_world.cpp) both include this, so a name
// added here reaches both and the two can never drift into different worlds.
//
// ---------------------------------------------------------------------------
// ALEXANDRE DUMAS' FRANCE — public domain (Dumas died in 1870).
//
// Chosen over invented filler because three novels share ONE France, so the
// relationships between these objects are REAL rather than random: the same
// king, the same Paris, the same regiment. Drawn from `Les Trois Mousquetaires`
// (1844), `Le Vicomte de Bragelonne` (1847-50, whose final part is known in
// English as `The Man in the Iron Mask`) and `Le Comte de Monte-Cristo` (1844).
//
// ⚠️ NAMES ARE FROM THE NOVELS. No modern adaptation, no translation-specific
// coinage, and no invented "expansion" material.
// ---------------------------------------------------------------------------

namespace scrivi::testcorpus {

const char* kCharacters[] = {
    // Les Trois Mousquetaires
    "d'Artagnan", "Athos", "Porthos", "Aramis", "Milady de Winter",
    "Cardinal Richelieu", "Monsieur de Tréville", "Constance Bonacieux",
    "Rochefort", "Planchet", "Grimaud", "Mousqueton", "Bazin",
    "Lord de Winter", "John Felton", "Kitty", "Monsieur Bonacieux",
    "Anne of Austria", "Louis XIII", "The Duke of Buckingham",
    "Bernajoux", "Jussac", "Cahusac", "Biscarat", "Lubin",
    // Vingt Ans Après / Le Vicomte de Bragelonne / The Man in the Iron Mask
    "Raoul de Bragelonne", "Louise de La Vallière", "Louis XIV", "Philippe",
    "Cardinal Mazarin", "Nicolas Fouquet", "Jean-Baptiste Colbert",
    "Charles II", "Mordaunt", "Madame de Chevreuse", "Olivier d'Herblay",
    "Monsieur du Vallon", "Comte de La Fère", "Blaisois", "Madame de Montalais",
    "Henrietta of England", "The Duke of Beaufort", "Baisemeaux",
    "Monsieur de Saint-Aignan", "Guiche", "Manicamp", "De Wardes",
    "Marie Michon", "Bertaudière", "Percerin", "Vanel",
    // Le Comte de Monte-Cristo
    "Edmond Dantès", "Abbé Faria", "Mercédès", "Fernand Mondego",
    "Danglars", "Gérard de Villefort", "Caderousse", "Maximilien Morrel",
    "Monsieur Morrel", "Julie Morrel", "Emmanuel Herbault", "Haydée",
    "Albert de Morcerf", "Madame de Villefort", "Valentine de Villefort",
    "Noirtier de Villefort", "Bertuccio", "Ali", "Baptistin", "Jacopo",
    "Luigi Vampa", "Peppino", "Benedetto", "Madame Danglars", "Eugénie Danglars",
    "Louise d'Armilly", "Lucien Debray", "Beauchamp", "Franz d'Épinay",
    "Château-Renaud", "Barrois", "Héloïse", "Countess G—", "Signor Pastrini",
    "Le Patron Baldi", "Gaetano", "Penelon", "Cocles", "Monsieur Boville",
};

const char* kLocations[] = {
    "Paris", "Meung-sur-Loire", "La Rochelle", "Armentières", "Béthune",
    "Marseille", "Château d'If", "Monte Cristo", "Rome", "Yanina",
    "Auteuil", "Fontainebleau", "Versailles", "Belle-Île-en-Mer", "Blois",
    "Le Havre", "Boulogne", "Calais", "London", "Portsmouth",
    "Amiens", "Chantilly", "Crèvecœur", "Lille", "Saint-Germain",
    "Vaux-le-Vicomte", "Pierrefonds", "Bragelonne", "La Fère", "Noisy-le-Sec",
    "Livorno", "Porto-Ferrajo", "Elba", "Marseille Harbour", "Catalans",
    "Rue Saint-Honoré", "Place Royale", "Pré-aux-Clercs", "Luxembourg", "Saint-Cloud",
};

const char* kBuildings[] = {
    "The Louvre", "The Bastille", "Palais-Cardinal", "Hôtel de Tréville",
    "The Convent of the Carmelites", "The Inn of the Jolly Miller",
    "Château de Vaux", "Château de Blois", "The Palais-Royal",
    "The Hôtel de Ville", "Villefort's House", "The Morrel Counting-House",
    "The Auteuil House", "The Colosseum", "The Inn of London",
    "The Bonacieux Lodgings", "The Red Dovecot", "The Carmes-Deschaux",
    "The Hôtel de Guise", "The Fortress of Belle-Île",
};

const char* kItems[] = {
    "The Queen's Diamond Studs", "The Cardinal's Carte Blanche",
    "The Spada Treasure", "Faria's Manuscript", "The Iron Mask",
    "Milady's Fleur-de-Lis", "The Poisoned Wine", "Dantès' Letter",
    "The Rope Ladder", "Athos' Sapphire Ring", "The Brandy Cask",
    "Porthos' Baldric", "The Bearer Bonds", "The Aqua Tofana",
    "The Telegraph Signal", "The Cardinal's Order", "Buckingham's Casket",
    "The Wedding Contract", "Noirtier's Dictionary", "The Yanina Report",
};

const char* kFactions[] = {
    "The King's Musketeers", "The Cardinal's Guards", "The Spanish Party",
    "The House of Bourbon", "The House of Habsburg", "The Roman Bandits",
    "The Carbonari", "The Marseille Shipowners", "The Jansenists",
    "The Fronde", "The English Court", "The Order of Saint-Michel",
};

const char* kVehicles[] = {
    "The Pharaon", "The Young Amelia", "The Count's Coach",
    "The Post-Chaise to Calais", "Buckingham's Cutter",
    "The Fishing Boat of Gaetano", "The Yacht of Monte Cristo",
    "The Prison Cart", "The Berline of Madame Danglars",
};

const char* kMaps[] = {
    "France under Louis XIII", "France under Louis XIV",
    "The Mediterranean", "The Isle of Monte Cristo",
    "Paris within the Walls", "The Siege of La Rochelle",
    "The Marseille Roadstead", "The Bastille Precinct",
};

const char* kChronicles[] = {
    "The Siege of La Rochelle", "The Affair of the Diamond Studs",
    "The Execution at Armentières", "The Arrest of Edmond Dantès",
    "The Escape from the Château d'If", "The Discovery of the Spada Treasure",
    "The Fall of Fouquet", "The Substitution at the Bastille",
    "The Death of Porthos", "The Ruin of the House of Morcerf",
    "The Poisonings at the Villefort House", "The Duel at the Pré-aux-Clercs",
    "The Restoration of Charles II", "The Fête at Vaux-le-Vicomte",
};

const char* kArtifacts[] = {
    "The Musketeer's Commission", "The Lettre de Cachet",
    "The Register of the Bastille", "The Spada Parchment",
    "The Deed to Monte Cristo", "The Marriage Register of Saint-Sulpice",
};

const char* kRules[] = {
    "The Edict against Duelling", "The Rights of a Musketeer",
    "The Law of Lettres de Cachet", "The Custom of the Sea",
    "The Rule of the Carmelites",
};

// Manuscript structure: chapters, each with scenes. ⚠️ The SCENE count is what
// the progress bar counts, so this is the knob that makes the fraction MOVE.
const char* kChapters[] = {
    "I. The Three Presents of Monsieur d'Artagnan",
    "II. The Antechamber of Monsieur de Tréville",
    "III. The Diamond Studs",
    "IV. The Siege of La Rochelle",
    "V. The Woman of Armentières",
    "VI. Marseille — the Arrival of the Pharaon",
    "VII. The Betrothal Feast",
    "VIII. The Château d'If",
    "IX. The Abbé and the Treasure",
    "X. The Count Returns to Paris",
    "XI. The House at Auteuil",
    "XII. The Fall of the House of Morcerf",
    "XIII. Twenty Years After",
    "XIV. The Fête at Vaux",
    "XV. The Man in the Iron Mask",
    "XVI. Belle-Île and the Death of a Giant",
};

const char* kSceneTitles[] = {
    "A Meeting on the Road", "The Challenge", "The Duel Interrupted",
    "An Audience", "The Queen's Errand", "Crossing to England",
    "The Bastion Saint-Gervais", "A Supper Under Fire", "The Judgement",
    "The Ship Comes In", "An Arrest at the Table", "The Dungeon",
    "A Voice Through the Wall", "The Sack and the Sea", "The Island",
    "A Card Left in Paris", "The Telegraph", "The Trial",
    "A Room in the Bastille", "The Substitution", "The Grotto",
    "The Last Charge", "A Letter Unsealed", "The Reckoning",
};
template <typename T, unsigned long N>
constexpr unsigned long countOf(T (&)[N]) { return N; }

}  // namespace scrivi::testcorpus
