#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ulfius.h>
#include <jansson.h>

#define PORT 8080
#define BOOK_ALLOC 5

int book_counter = 0;

typedef struct {
    char *name, *gender;
    int year, id;
} _book;

int greeting(const struct _u_request * request, struct _u_response * response, void * user_data) {
    ulfius_set_string_body_response(response, 200, "Hello dev!");
    return U_CALLBACK_COMPLETE;
}

int get_book_by_id(const struct _u_request * request, struct _u_response * response, void * user_data) {
    char *id = (char *) u_map_get(request->map_url, "id");
    if(id == NULL) {
        printf("failed to get the book id on URL");
        return -1;
    }
    json_t *response_json = json_object();
    int book_id = atoi(id);
    if(book_id > book_counter || book_id < 0) {
        json_object_set_new(response_json, "message", json_string("unexistent book id!"));
        ulfius_set_json_body_response(response, 404, response_json);
        return U_CALLBACK_COMPLETE;
    }
    _book *books = (_book *) user_data;
    _book book = books[book_id];

    json_object_set_new(response_json, "name", json_string(book.name));
    json_object_set_new(response_json, "gender", json_string(book.gender));
    json_object_set_new(response_json, "year", json_integer(book.year));

    ulfius_set_json_body_response(response, 200, response_json);
    
    return U_CALLBACK_COMPLETE;
}

int create_book(const struct _u_request * request, struct _u_response * response, void * user_data) {
    _book *books = (_book *) user_data;
    json_t *request_json = ulfius_get_json_body_request(request, NULL);
    if(request_json == NULL) {
        printf("Failed to get the response json on /book POST");
    }

    char *name = (char *) json_string_value(json_object_get(request_json, "name"));
    char *gender = (char *) json_string_value(json_object_get(request_json, "gender"));
    int year = json_integer_value(json_object_get(request_json, "year"));

    if(name == NULL) {
        json_t *json = json_object();
        json_object_set_new(json, "message", json_string("Name field required!"));
        ulfius_set_json_body_response(response, 400, json);
        return U_CALLBACK_COMPLETE;
    }

    if(gender == NULL) {
        json_t *json = json_object();

        json_object_set_new(json, "message", json_string("Gender field required!"));
        ulfius_set_json_body_response(response, 400, json);
        return U_CALLBACK_COMPLETE;
    }

    if(year == 0) {
        json_t *json = json_object();

        json_object_set_new(json, "message", json_string("Year field required"));
    }

    books[book_counter].id = book_counter;
    books[book_counter].name = name;
    books[book_counter].gender = gender;
    books[book_counter].year = year;
    book_counter++;

    json_t *response_json = json_object();
    json_object_set_new(response_json, "Message", json_string("Book added with success"));
    
    ulfius_set_json_body_response(response, 200, response_json);
    return U_CALLBACK_COMPLETE;
}

int update_book(const struct _u_request * request, struct _u_response * response, void * user_data) {
    char * id = (char *) u_map_get(request->map_url, "id");
    if(id == NULL) {
        printf("Failed to find the book by id on update_book");
    }

    json_t *response_json = json_object();
    int book_id = atoi(id);
    if(book_id > book_counter || book_id < 0) {
        json_object_set_new(response_json, "message", json_string("unexistent book id"));
        ulfius_set_json_body_response(response, 404, response_json);
        return U_CALLBACK_COMPLETE;
    }
    _book *books = (_book *) user_data;
    json_t *request_json = ulfius_get_json_body_request(request, NULL);
    if(request_json == NULL) {
        printf("request Json non existent on update book");
    } 

    char *name = (char *) json_string_value(json_object_get(request_json, "name"));
    if(name != NULL) {
        books[book_id].name = name;
    }

    char *gender = (char *) json_string_value(json_object_get(request_json, "gender"));
    if(gender != NULL) {
        books[book_id].gender = gender;
    }

    int year = json_integer_value(json_object_get(request_json, "year"));
    if(year != 0) {
        books[book_id].year = year;
    }

    json_object_set_new(response_json, "message", json_string("Book updated with success"));
    ulfius_set_json_body_response(response, 200, request_json);

    return U_CALLBACK_COMPLETE;
}

int main(void) {
    struct _u_instance instance;
    _book *books = (_book *) malloc(BOOK_ALLOC * sizeof(_book));

    ulfius_init_instance(&instance, PORT, NULL, NULL);

    ulfius_add_endpoint_by_val(&instance, "GET", "", NULL, 0, &greeting, NULL);

    ulfius_add_endpoint_by_val(&instance, "GET", NULL, "/book/@id", 0, &get_book_by_id, books);

    ulfius_add_endpoint_by_val(&instance, "POST", "book", "", 0, &create_book, books);

    ulfius_add_endpoint_by_val(&instance, "PUT", NULL, "book/@id", 0, &update_book, books);

    if(ulfius_start_framework(&instance) == U_OK) {
        printf("Server running on port %d...\n", instance.port);

        getchar();
    };

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);
}