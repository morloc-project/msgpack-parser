dyn.load("~/.morloc/lib/libmpackr.so")

pack <- function(obj, schema) {
    .Call("_mlcmpack_r_pack", obj, schema)
}

unpack <- function(packed, schema) {
    .Call("_mlcmpack_r_unpack", packed, schema)
}

# Function to print colored text
color_text <- function(text, color) {
    colors <- list(
        red = "\033[31m",
        green = "\033[32m",
        reset = "\033[0m"
    )
    paste(colors[[color]], text, colors[["reset"]], sep="")
}

# Function to compare objects
compare_objects <- function(obj1, obj2) {
    if (is.list(obj1) && is.list(obj2)) {
        if (length(obj1) != length(obj2)) return(FALSE)
        return(all(mapply(compare_objects, obj1, obj2)))
    } else if (is.atomic(obj1) && is.atomic(obj2)) {
        return(all(obj1 == obj2))
    } else {
        return(FALSE)
    }
}

test_cases <- list(
    list("Test boolean", "b", TRUE),
    list("Test float", "f", 3.14),
    list("Test integer", "i4", 14L),
    list("Test string", "s", "Hello"),
    list("Test raw binary", "r", as.raw(c(0x01, 0x02, 0x03))),
    list("Test array of integers", "ai4", c(1L,2L,3L,4L,5L)),
    list("Test array of floats", "af8", c(1.0, 2.0, 3.0)),
    list("Test array of booleans", "ab", c(TRUE,FALSE,FALSE)),
    list("Test array of arrays of booleans", "aab", list(c(TRUE,FALSE), c(FALSE,FALSE,TRUE))),
    list("Test tuple of int and array of floats", "t2i4af8", list(42L, c(1.1, 2.2, 3.3))),
    list("big numeric vector", "af8", runif(1000000))
)

for (case in test_cases) {
    test_description <- case[[1]]
    schema_str <- case[[2]]
    original_data <- case[[3]]

    tryCatch({
        msgpack_data <- pack(original_data, schema_str)
        returned_data <- unpack(msgpack_data, schema_str)

        if (compare_objects(original_data, returned_data)) {
            cat(test_description, "...", color_text("pass", "green"), "\n")
        } else {
            cat(test_description, "...", color_text("fail", "red"), "\n")
            cat("Original:", toString(original_data), "\n")
            cat("Returned:", toString(returned_data), "\n")
        }
    }, error = function(e) {
        cat(test_description, "...", color_text("ERROR", "red"), "\n")
        cat("Error message:", e$message, "\n")
    })
}
