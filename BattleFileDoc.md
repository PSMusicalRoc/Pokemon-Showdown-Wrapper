# Battle Files

A battle file is a file used for importing CPU battles into this program, having the extension `.pkteam`. They are broken up into two sections, separated by a `#[sectionname]` header:
1. A JSON object containing information about the trainer itself (name, sprite, class, etc), the battle's format, and any other important initialization data. This will be right after `#trainerdata`.
2. A PokePaste string containing the team for the trainer to use. PokePaste is used instead of a more streamlined language (packed format, JSON, etc), so that the contents are easily edited on, imported to, and exported from Pokemon Showdown's website. It's also much more human readable, an important aspect for a customizable program. This will appear after `#team`.

### TrainerData Parameters

```json
{
    "name": "A string containing the trainer's display name",
    "class": "A string containing the trainer's class",
    "format": "A string containing the pokemon showdown format the trainer will use in the fight",
    "teamsize": 3, // An integer (from 1 to 6) detailing the number of pokemon the trainer will use
    "sprite": "" // Unknown at the moment. Implementation is not finalized for spritework yet.
}
```

### Team

As stated, this will be a standard PokePaste. Since the underlying engine is Pokemon Showdown, any PokePaste should work out of the box.