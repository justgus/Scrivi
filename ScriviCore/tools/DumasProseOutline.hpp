#pragma once
// DumasProseOutline.hpp — SP-128 / [I-0196] — the PROSE variant's outline.
//
// ⚠️ STAGE 1 OF N. This file is the OUTLINE ONLY: one thesis line per chapter and
// one topic line per scene. ⚠️ The full prose is added CHAPTER BY CHAPTER in
// later stages, so no single step has to carry 50,000 words at once.
//
// ---------------------------------------------------------------------------
// WHY A PROSE VARIANT EXISTS
//
// ⚠️ The default fixture creates 1,153 scenes whose bodies are ZERO BYTES.
// ✅ MEASURED on it: a project open moved ~1.2 MB in 321 s — about 3.8 KB/s.
// ⚠️ The load is NOT I/O-bound; it is spending essentially all its time
// re-walking directory trees. ⚠️ SO 321 s IS THE FLOOR: it is what the app costs
// when there is NOTHING TO READ.
//
// ⚠️ The user's point, and it is the reason this file exists: real scenes will
// have content, and then the traversal cost is paid ON TOP of real I/O.
//   - the 251 s timeline half is UNCHANGED by prose (it reads sidecars only)
//   - the 70 s scene-load half gains real body reads
//   - Spotlight's extract, which strips every body on EVERY open AND EVERY app
//     resign, starts stripping real text instead of empty files
//
// ---------------------------------------------------------------------------
// ⚠️ STRUCTURE IS FIXED AT 16 CHAPTERS x 24 SCENES = 384 SCENES.
//
// ⚠️ DO NOT "improve" this by using fewer, longer scenes. ✅ The 384/1152 scene
// counts are the ones the 321 s baseline was measured against, and [I-0196]'s
// cost scales with SCENE COUNT, not word count. ⚠️ Changing the shape would make
// the before/after comparison meaningless.
//
// ✅ 50,000 words / 384 scenes ≈ 130 words per scene (~0.29 MB at scale 1,
// ~0.9 MB at scale 3).
//
// ---------------------------------------------------------------------------
// THE SUBJECT (user's brief): a critical essay on Alexandre Dumas' characters —
// public domain, and a genuine subject rather than lorem ipsum, so word-boundary
// and text-stripping code meets real prose.
//
//   PART ONE   (ch. I–IV)    Compare and contrast across the storylines:
//                            parallel heroes, parallel villains, the rebellious
//                            temperament, and "fighting the establishment".
//   PART TWO   (ch. V–VIII)  The same character grown older treated as a NEW
//                            character — Musketeers vs. Bragelonne/Iron Mask.
//   PART THREE (ch. IX–XII)  As written vs. their vast presence in film and
//                            television.
//   PART FOUR  (ch. XIII–XVI) As ARCHETYPES that reached comics, science
//                            fiction and other genre media.

namespace scrivi::testcorpus::prose {

// One thesis per chapter. Stage 2+ expands each into 24 scenes of ~130 words.
struct ChapterOutline {
    const char* title;
    const char* thesis;
};

inline constexpr ChapterOutline kChapterOutlines[] = {
  // ---- PART ONE — the parallel cast -------------------------------------
  {"I. Two Debuts, One Temperament",
   "d'Artagnan arrives in Paris with a yellow horse and no money; Edmond Dantes "
   "arrives in Marseille with a command and a betrothal. Both are provincials "
   "walking into a system that has already decided what they are worth."},
  {"II. The Villain as Instrument",
   "Richelieu and Villefort are not monsters but functionaries. Dumas' most "
   "durable villains are men doing their jobs, which is why they outlast the "
   "duellists who oppose them."},
  {"III. Milady and the Limits of the Type",
   "Milady de Winter is the exception that exposes the rule: the only antagonist "
   "Dumas grants an interior life, and the only one the heroes must judge rather "
   "than merely defeat."},
  {"IV. Fighting the Establishment, Serving the King",
   "The musketeers rebel constantly and obey absolutely. The contradiction is not "
   "a flaw in the writing; it is the engine of every plot Dumas builds."},

  // ---- PART TWO — the same man, older, as a new character ---------------
  {"V. Athos and the Comte de la Fere",
   "The drunk with a secret becomes a landed father with a conscience. Dumas "
   "does not age Athos so much as replace him, and expects the reader not to "
   "notice."},
  {"VI. Porthos, or Strength Without a Question",
   "Alone among the four, Porthos does not change — and his death at Belle-Ile "
   "lands hardest precisely because he never learned to doubt."},
  {"VII. Aramis Becomes the Plot",
   "The abbe who duels his way through book one becomes the man who moves a king. "
   "Aramis is the clearest case of a character promoted from participant to "
   "author of events."},
  {"VIII. d'Artagnan as Institution",
   "The boy who fought everyone becomes the captain who enforces order. The "
   "rebel's reward is to become the establishment he defied."},

  // ---- PART THREE — as written vs. as filmed ----------------------------
  {"IX. The Swashbuckler That Dumas Did Not Write",
   "Film inherited the sword and discarded the politics. The page spends more "
   "time on letters and money than on blades."},
  {"X. The Count on Screen",
   "Adaptation almost always shortens the imprisonment and lengthens the revenge, "
   "inverting the proportion that gives the novel its weight."},
  {"XI. The Mask Becomes the Story",
   "A subplot in the final third of a long novel became, on screen, the title and "
   "the whole. The image outgrew its text."},
  {"XII. Casting as Interpretation",
   "Each era recasts d'Artagnan as what it wants youth to be: gallant, comic, "
   "brooding, ironic. The character is a mirror with a sword."},

  // ---- PART FOUR — the archetypes downstream ----------------------------
  {"XIII. The Found Family, Armed",
   "Four temperaments bound by an oath is a template that runs through team "
   "comics, heist films and space opera crews largely unaltered."},
  {"XIV. The Wronged Man Who Returns Wealthy",
   "Dantes' pattern — false imprisonment, hidden fortune, assumed identity, "
   "measured revenge — recurs wherever a genre needs a patient avenger."},
  {"XV. The Face Behind the Mask",
   "Concealed identity as both plot and metaphor: the mask that hides a claim to "
   "the throne became the mask that hides a claim to the self."},
  {"XVI. Why These Four Survived",
   "Dumas' characters persist because they are types with contradictions — "
   "specific enough to recognise, loose enough to reinhabit."},
};

// The 24 scene topics repeat per chapter, so every chapter examines its thesis
// from the same 24 angles. ⚠️ Deliberately uniform: the fixture must be
// PREDICTABLE, and a scene's length must not depend on which chapter it is in.
inline constexpr const char* kSceneTopics[] = {
    "The claim stated plainly",
    "What the text actually says",
    "A passage read closely",
    "The counter-example",
    "What the adaptation changed",
    "The historical setting",
    "The class question",
    "The money question",
    "Loyalty as a plot device",
    "The duel as argument",
    "Women in the frame",
    "Servants and the invisible cast",
    "The letter as weapon",
    "Institutions and their agents",
    "Youth against age",
    "The oath and its cost",
    "Comedy inside the danger",
    "The scene everyone remembers",
    "The scene nobody films",
    "What the sequel retconned",
    "The archetype's later life",
    "An objection considered",
    "What survives translation",
    "Summary and the next question",
};

}  // namespace scrivi::testcorpus::prose
