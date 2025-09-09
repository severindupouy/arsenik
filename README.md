This repository is a fork of the _marvelous_ [Arsenik](https://github.com/OneDeadKey/arsenik) repository.

Thanks for the ingenious and great work!

Take a look at the original repository to find full documentation.

---

Here, you can find various configurations I made for my personal use.

My setup :

- a laptop running manjaro with **sway-wm**
- a **typematrix** keyboard at work (<- kanata)
- a **piantor** keyboard at home (<- qmk)
- **ergol** layout
- **HRM** activated
- **arsenik** activated

_Note : When I don't have one of my external keyboards plugged in, I still use the ergol layout. However, I keep azerty enabled in case I might need it._

---

## Table of contents

- [QMK new arsenik generator script](#updated-qmk-keymap-generator-script)
- [QMK new arsenik generator documentation](#updated-documentation)
- [QMK my arsenik ergol+HRM flavor](#arsenik--ergolhrm-flavor)
  - [<kbd>\_nav_layer</kbd> breaking changes](#_nav_layer-breaking-changes)
    - replacing `KC_TAB` by `G(S(KC_Q))`
    - replacing `C(AS(A))` by `KB_TAB`
    - replacing right hand numbers pad by fn keys
  - [new <kbd>\_media_layer</kbd>](#new-_media_layer)
  - [new <kbd>\_mouse_layer</kbd>](#new-_mouse_layer)
- [KANATA : bépo typematrix on laptop](##kanata--b%C3%A9po-typematrix-on-laptop)

---

## QMK

### new features

#### updated qmk keymap generator script

I made a complete rewrite of the script to generate the keymap with another logic :

- the 2 main features can be run separately (cli flags `--generate`, `--copy`)
- idempotent script (you can run it multiple times)
- logs
- remove automatic opening of the editor
- reorganize folder structure

#### updated documentation

I made a complete rewrite of the [qmk/readme.md](/qmk/readme.md) to match the new logic (of mine).

I tried to make the explanations as accessible as possible, in order to help beginners (like I was) understand what “arsenik” is used for and how it works.

### piantor config with tweaks

Piantor is a 3x6 split keyboard with 2x3 thumbs clusters. It affords to expand easily the number of layers.

#### arsenik : ergol+HRM flavor

As I understand and support the motives of the original arsenik choices, specifically to be the more generic as possible, I tried to make some changes to fit my needs. Of course, on that path, I loose some genericity in comparison with the original.

So, here are the major differences from the original arsenik layout.

##### <kbd>\_nav_layer</kbd> (breaking changes)

- replacing `KC_TAB` by `G(S(KC_Q))`

> Motivations :
>
> - as I found that `Meta+Shift+Q` wasn't so practical on ergol with HRM, I was looking for a **better shortcut to close a window on sway**
> - In sway, the official shortcut is `Meta+Shift+Q`. I could have chosen to change it in sway configuration, but I prefered to stay with this default.
> - as many software can be closed with `Ctrl+Q` (not all of them), I makes sense to have `Meta+Shift+Q` on <kbd>\_nav_layer</kbd>, at the same place as <kbd>Q</kbd> in <kbd>\_base_layer</kbd>

- replacing `C(AS(A))` by `KB_TAB`

> Motivations :
>
> - as I removed the `KB_TAB` from its place, I had to **find another place**.
> - **`C(AS(A))` can be completely removed** as you can use `Ctrl+A` directly on <kbd>\_base_layer</kbd> (with ergol+HRM)
> - Additionnally I found it could make sense to put the **`tab` key on the home row** to match the increasingly intensive use of this key by **AI-related features in IDEs**.

- on right hand, replace all numbers by fn keys

> Motivations :
>
> I experienced not using at all this numbers pad
> to write numbers, I prefer to use of the <kbd>\_num_layer</kbd>
> I have a big use of F2 and F5, so I was looking for a good place for them
> removing all numbers makes a lot of new places, perfect for fn keys

##### new <kbd>\_media_layer</kbd>

> Motivations :
>
> - I wanted to have access to some media keys (PLAY/STOP/BRIGHTNESS, etc.)
> - I also put there extra **F2 and F5 keys, to have them on left hand**

##### new <kbd>\_mouse_layer</kbd>

> Motivations :
>
> I wanted to experiment to control mouse by the keyboard. In fact, I never do it.

#### renaming layers

```
enum arsenik_layers {
    _base,
    _lafayette,
    _num_layer,   // renamed
    _vim_layer,   // removed
    _nav_layer,
    _fn_layer,    // removed
    _media_layer, // new
    _mouse_layer, // new
};
```

## KANATA : bépo typematrix on laptop

Some extra configuration to fit my setup :

- narrow kanata activation : only for typematrix, not the laptop keyboard
- a **tweaked navigation layer** (mirror of the one I use on qmk). See [kanata/deflayer/navigation.kbd](kanata/deflayer/navigation.kbd))
  - on right hand, full access to fn keys instead of numbers
  - on left hand, access to mwheel-up functions instead of copy/paste/...
- a shortcut to close the window on SWAY-WM (re-mapping of `MOD+SHIFT+Q` which is not really easy to do on ergol)
