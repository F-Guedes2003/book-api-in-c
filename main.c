#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ulfius.h>
#include <jansson.h>

#define PORT 8080

int greeting(const struct _u_request * request, struct _u_response * response, void * user_data) {
    ulfius_set_string_body_response(response, 200, "Hello dev!");
    return U_CALLBACK_COMPLETE;
}

int main(void) {
    struct _u_instance instance;

    ulfius_init_instance(&instance, PORT, NULL, NULL);

    ulfius_add_endpoint_by_val(&instance, "GET", "", NULL, 0, &greeting, NULL);

    if(ulfius_start_framework(&instance) == U_OK) {
        printf("Server running on port %d...\n", instance.port);

        getchar();
    };

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);
}