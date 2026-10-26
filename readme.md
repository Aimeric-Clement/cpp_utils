This project is mostly intended for copy pasting template utilities back into other projects.
I find that as of cpp17, std::variants aren't that well supported, with quite a lot of features that are not easily available such as going from: `std::variant<Dog, Cat, Car>` to `std::variant<Dog, Cat>` conveniently, or having a convenient to_string that works with most variants out of the box for debugging. std::variant became an important part of the language, but the standard didn't went all the way with the more convenient QoL features yet.

template_utils, variant_utils, are adressing some of these.

Note that I consider a variant that repeats types (e.g. `std::variant<int, int>`) to be mostly a mistake, and thus you may notice none of these utilities consider that edge-case (asside sometimes where it is guarding against it).