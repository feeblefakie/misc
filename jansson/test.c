#include <stdio.h>
#include <jansson.h>

int main(void)
{
  json_t *json;
  json_error_t error;

  json = json_load_file("./q3.json", 0, &error);
  if (!json) {
    fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
    free(json);
    return 1;
  }

  json_t *rels = json_object_get(json, "rels");
  json_t *rel;

  int i;
  json_array_foreach(rels, i, rel) {
    printf("%s\n", json_string_value(json_object_get(rel, "id")));
    printf("%s\n", json_string_value(json_object_get(rel, "relOp")));
  }

  free(json);
}
