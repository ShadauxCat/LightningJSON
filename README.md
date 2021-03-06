# LightningJSON

LightningJSON is a C++ JSON parser written with the goal of maximizing performance of both parsing and lookups. It has one of the fastest JSON parsers available (especially when parsing with strict mode disabled) and its runtime data structures are built on fast algorithms such as [SkipProbe](https://github.com/ShadauxCat/SkipProbe) and a thread-local pool allocator to achieve fast allocations of dynamic memory, which is used as little as possible.

To ensure parsing is as quick as possible, LightningJSON parses the JSON string in-place and will hold references to the string for as long as the LightningJSON object exists. Type conversions are processed lazily, meaning the string is not converted into an integer, boolean, float, or double until accessed that way. Likewise, strings aren't unescaped until requested. LightningJSON only spends its time doing what you actually need it to do - the only thing it does in the initial parse is parse the overall structure (objects and arrays) and collect type metadata.

Proper documentation and benchmarks will come soon.

Basic usage:

```c++
LightningJSON::JSONObject foo = LightningJSON::JSONObject::FromString(std::string_view(jsonString.c_str(), jsonString.length());
double d = foo["bar"]["baz"][3].AsDouble();
foo["bar"]["baz"][3] = 1.5;
std::string output = foo.ToJSONString(true);
```