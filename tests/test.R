dyn.load("lib/librmorloc.so")

pack <- function(obj, schema) {
    .Call("to_mesgpack", obj, schema)
}

unpack <- function(packed, schema) {
    .Call("from_mesgpack", packed, schema)
}

shm_start <- function(shm_basename, shm_size){
    .Call("shm_start", shm_basename, shm_size)
}

shm_close <- function(){
    .Call("shm_close")
}

to_shm <- function(x, schema_str){
    .Call("to_shm", x, schema_str)
}

from_shm <- function(relptr, schema_str){
    .Call("from_shm", relptr, schema_str)
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
        return(all.equal(obj1, obj2, tolerance=0.00001))
    } else {
        return(FALSE)
    }
}

test_cases <- list(
    # bools
    list("Test boolean", "b", TRUE),

    list("Test booleans", "ab", c(TRUE, FALSE)),
    list("Test booleans list", "ab", list(TRUE, FALSE), c(TRUE, FALSE)),

    # floats
    list("Test f4", "f4", 3.14),
    list("Test f8", "f8", 3.14),
    list("Test floats", "af8", c(3.14, 5.6)),
    list("Test floats", "af8", list(3.14, 5.6), c(3.14, 5.6)),

    # ints
    list("Test i1", "i1", 14L),
    list("Test i2", "i2", 14L),
    list("Test i4", "i4", 14L),
    list("Test i8", "i8", 14L),
    list("Test large i8", "i8", 2**50),

    list("Test -i1", "i1", -14L),
    list("Test -i2", "i2", -14L),
    list("Test -i4", "i4", -14L),
    list("Test -i8", "i8", -14L),
    list("Test -large i8", "i8", -2**50),

    list("Test u1", "u1", 14L),
    list("Test u2", "u2", 14L),
    list("Test u4", "u4", 14L),
    list("Test u8", "u8", 14L),
    list("Test large u8", "u8", 2**50),

    list("Test empty array of i4", "ai4", integer(0)),
    list("Test empty array of i8", "ai4", integer(0)),
    list("Test empty array of f4", "af4", numeric(0)),
    list("Test empty array of f8", "af4", numeric(0)),

    list("Test integers", "ai4", c(14L, 15L), c(14, 15)),
    list("Test integers list", "ai4", list(14L, 15L), c(14L, 15L)),
    list("Test integer from double", "i4", 14, 14L),
    list("Test integers from doubles", "ai4", c(14, 15), c(14L, 15L)),
    list("Test integers list from doubles", "ai4", list(14, 15), c(14L, 15L)),

    # arrays
    list("Test random integers", "ai8", sample.int(2**51, 1000000)),
    list("Test random bool", "ab", sample.int(2, 1000000, replace=T) == 1),
    list("Test long random character", "as", as.character(sample.int(2**51, 100000))),

    # strings
    list("Test string", "s", "Hello"),
    list("Test random long string", "s", paste(as.character(sample.int(2**51, 2)), collapse="")),
    list("Test strings", "as", c("Hello", "Goodbye")),
    list("Test strings list", "as", list("Hello", "Goodbye"), c("Hello", "Goodbye")),
    # binary
    list("Test empty raw binary", "au1", raw(0)),
    list("Test raw binary", "au1", as.raw(c(0x01, 0x02, 0x03))),
    list("Test long raw binary", "au1", as.raw( sample.int(256, 100000, replace=TRUE) - 1 )),
    list("Test raw binaries", "aau1", list(as.raw(c(0x01, 0x02, 0x03)), as.raw(c(0x00, 0x01)))),
    # lists
    list("Test array of arrays of booleans", "aab", list(c(TRUE,FALSE), c(FALSE,FALSE,TRUE))),
    list("big numeric vector", "af8", runif(1000000)),
    # tuples
    list("Test tuple of int and array of floats", "t2i4af8", list(42L, c(1.1, 2.2, 3.3))),
    list("tuple of lists", "t2asaai4", list(c("bad", "john"), list(c(1L,2L), c(4L,5L,6L)))),
    # maps
    list("map", "m21ab1bi4", list(a = T, b = 42)),
    list("list of maps", "am21ab1bi4", list(list(a = T, b = 42), list(a = F, b = 420)))
)


nfails <- 0
ntotal <- length(test_cases)

shm_start("rtest", 0x100)

for (case in test_cases) {
    test_description <- case[[1]]
    schema_str <- case[[2]]
    original_data <- case[[3]]

    tryCatch({
        msgpack_data <- pack(original_data, schema_str)
        returned_data <- unpack(msgpack_data, schema_str)

        if (length(case) == 4){
          expected_data <- case[[4]]
        } else {
          expected_data <- original_data
        }

        if (compare_objects(expected_data, returned_data)) {
            cat(test_description, "...", color_text("pass", "green"), "\n")
        } else {
            nfails <- nfails + 1
            cat(test_description, "...", color_text("fail", "red"), "\n")
            cat("Original:", toString(expected_data), " where class =", class(expected_data), "\n")
            cat("Returned:", toString(returned_data), " where class =", class(returned_data), "\n")
        }
    }, error = function(e) {
        nfails <<- nfails + 1
        cat(test_description, "...", color_text("fail", "red"), "\n")
        cat("Error message:", e$message, "\n")
    })
}

cat(nfails, "/", ntotal, " failed\n")

shm_close()
