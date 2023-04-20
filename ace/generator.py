ITERATION_COUNT = 150

TEXT = """
$Vector2: struct {
    x: int,
    y: int,
}
$compound(): int {
    value: $Vector2 = new $Vector2{ x: 3, y: -27 };
    value.y -= 1;
    print_int(value.y);
    boxed_value: *$Vector2 = box value;
    boxed_value.x *= 2;
    print_int(boxed_value.x);
    ret 0;
}
$precedence(): int {
    i: int = -$value();
    print_int(i);
    ret 0;
}
$value(): int {
    ret -131255;
}
$assoc(): int {
    ret 1 * -100 + 50 / 10 / 5;
}
$Getter: struct {
    field: int,
}
impl $Getter {
    field(): int -> self {
        ret self.field;
    }
}
$getter(): int {
    print_int(15);
    obj: $Getter = new $Getter { field: $value() };
    print_int(60);
    ret obj.field();
}
"""

IDENTIFIERS = [
    "Vector2",
    "compound",
    "precedence",
    "value",
    "assoc",
    "Getter",
    "getter",
]

def main():
    full_text = ""
    for i in range(ITERATION_COUNT):
        text = TEXT
        for j in range(len(IDENTIFIERS)):
            identifier = "$" + IDENTIFIERS[j]
            new_identifier = "id_" + str(i) + "_" + str(j)
            text = text.replace(identifier, new_identifier)
        full_text += text
    path = "src/generated.ace"
    with open(path, "w") as file:
        file.write(full_text)
    print("Written", full_text.count("\n"), "lines to", path)

if __name__ == "__main__":
    main()
