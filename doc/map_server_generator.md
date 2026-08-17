# Map Server Generator

This is a tool to generate files that are hard to manually create.

## How to run
### Linux
Run `make tools`.
This creates a new binary called `map-server-generator`.

It can be ran with: `./map-server-generator`

### Windows
It can be ran with `./map-server-generator.exe`, or with the provided `.bat` files.

## Available options
On Linux, prefix with `--`

On Windows, prefix with `/`

option | feature
---|---
`generate-navi` | create navigation files
`generate-reputation` | create reputation bson files
`generate-itemmoveinfo` | create itemmoveinfov5.txt
`no-comment` | omit the human-readable annotations in the navi distance tables

`no-comment` is a modifier, not a target: it never satisfies the "you must set
at least one option" check on its own.

The navi distance tables end every row with an annotation such as
`-- ReachableFromDst warp (alberta, 15, 234)`. It is there for whoever reads the
generated file by hand, and it is expensive: on a full Moonlight generation it
accounts for 68% of `navi_linkdistance` and 47% of `navi_npcdistance`, or 18.3
MiB shrinking to 7.1 MiB. The client re-parses all of it as Lua on every startup,
so dropping it costs nothing at runtime and saves both GRF space and load time.
Keep the comments when you intend to diff or debug the tables by hand.


