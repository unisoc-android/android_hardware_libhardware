/*
 * Copyright (C) 2019 Unisoc Project
 * 
 * History:
 *     <Date> 2019/04/12
 *
 */

#ifndef ANDROID_INCLUDE_HARDWARE_FACE_H
#define ANDROID_INCLUDE_HARDWARE_FACE_H

#include <hardware/hw_auth_token.h>

#define FACE_MODULE_API_VERSION_1_0 HARDWARE_MODULE_API_VERSION(1, 0)
#define FACE_HARDWARE_MODULE_ID "face"

typedef enum face_status {
    FACE_OK = 0, /* The method was invoked successfully. */
    FACE_ILLEGAL_ARGUMENT = 1, /* One of the arguments to the method call is invalid. */
    FACE_OPERATION_NOT_SUPPORTED = 2, /* This face HAL does not support this operation. */
    FACE_INTERNAL_ERROR = 3, /* The HAL has encountered an internal error and cannot complete the request. */
    FACE_NOT_ENROLLED = 4, /* The operation could not be completed because there are no enrolled templates. */
} face_status_t;

typedef enum face_msg_type {
    FACE_ERROR = -1,
    FACE_ACQUIRED = 1,
    FACE_TEMPLATE_REMOVED = 2,
    FACE_TEMPLATE_ENROLLING = 3,
    FACE_AUTHENTICATED = 4,
    FACE_TEMPLATE_ENUMERATED = 5,
    FACE_LOCKOUT_CHANGED = 6,
    FACE_ENROLL_PROCESSED = 7,
    FACE_AUTHENTICATE_PROCESSED = 8,
} face_msg_type_t;

/*
 * Face errors are meant to tell the framework to terminate the current operation and ask
 * for the user to correct the situation. These will almost always result in messaging and user
 * interaction to correct the problem.
 */
typedef enum face_error {
    FACE_ERROR_HW_UNAVAILABLE = 1, /* The hardware has an error that can't be resolved. */
    FACE_ERROR_UNABLE_TO_PROCESS = 2, /* Bad data; operation can't continue. */
    FACE_ERROR_TIMEOUT = 3, /* The operation has timed out waiting for user input. */
    FACE_ERROR_NO_SPACE = 4, /* No space available to store a template. */
    FACE_ERROR_CANCELED = 5, /* The current operation can't proceed. See above. */
    FACE_ERROR_UNABLE_TO_REMOVE = 6, /* face with given id can't be removed. */
    FACE_ERROR_LOCKOUT = 7, /* soft lockout for too many unsuccessful attempts. */
    FACE_ERROR_LOCKOUT_PERMANENT = 8, /* Face authentication is disabled until the user unlocks with strong authentication. */
    FACE_ERROR_VENDOR_BASE = 1000, /* vendor-specific error messages start here. */
    FACE_ERROR_AUTH_LIVENESSFAIL,
    FACE_ERROR_AUTH_NOFACE,
    FACE_ERROR_AUTH_FAIL,
    FACE_ERROR_VERIFY_TOKEN_FAIL,
    FACE_ERROR_AUTH_NOTEMPLATE, /* Algo version is different and images are not available */
} face_error_t;

/*
 * Face acquired info is meant as feedback for the current operation.
 */
typedef enum face_acquired {
    FACE_ACQUIRED_GOOD,
    FACE_ACQUIRED_INSUFFICIENT,
    FACE_ACQUIRED_TOO_BRIGHT,
    FACE_ACQUIRED_TOO_DARK,
    FACE_ACQUIRED_TOO_CLOSE,
    FACE_ACQUIRED_TOO_FAR,
    FACE_ACQUIRED_FACE_TOO_HIGH,
    FACE_ACQUIRED_FACE_TOO_LOW,
    FACE_ACQUIRED_FACE_TOO_RIGHT,
    FACE_ACQUIRED_FACE_TOO_LEFT,
    FACE_ACQUIRED_POOR_GAZE,
    FACE_ACQUIRED_NOT_DETECTED,
    FACE_ACQUIRED_TOO_MUCH_MOTION,
    FACE_ACQUIRED_RECALIBRATE,
    FACE_ACQUIRED_TOO_DIFFERENT,
    FACE_ACQUIRED_TOO_SIMILAR,
    FACE_ACQUIRED_PAN_TOO_EXTREME,
    FACE_ACQUIRED_TILT_TOO_EXTREME,
    FACE_ACQUIRED_ROLL_TOO_EXTREME,
    FACE_ACQUIRED_FACE_OBSCURED,
    FACE_ACQUIRED_START,
    FACE_ACQUIRED_SENSOR_DIRTY,
    FACE_ACQUIRED_VENDOR_BASE = 1000,
    FACE_ACQUIRED_MULTI_FACE = FACE_ACQUIRED_VENDOR_BASE,
    FACE_ACQUIRED_LIVENESS_FAIL,
    FACE_ACQUIRED_OUT_OF_IMAGE,
    FACE_ACQUIRED_AE_NOT_CONVERGED,
    FACE_ACQUIRED_TILT_TOO_HIGH,
    FACE_ACQUIRED_TILT_TOO_LOW,
    FACE_ACQUIRED_TILT_TOO_RIGHT,
    FACE_ACQUIRED_TILT_TOO_LEFT,
} face_acquired_t;

typedef struct face_enroll {
    uint32_t fid;
} face_enroll_t;

typedef struct face_removed {
    uint32_t fid;
} face_removed_t;

typedef struct face_authenticated {
    uint32_t fid;
    hw_auth_token_t hat;
} face_authenticated_t;

typedef struct face_enumerated {
    uint32_t fid;
} face_enumerated_t;

typedef struct face_lockout {
    uint64_t duration;
} face_lockout_t;

typedef struct face_enroll_processed {
    int64_t addr;        // the address of the processed frame buffer
    uint32_t remaining;
} face_enroll_processed_t;

typedef struct face_authenticate_processed {
    int64_t main;        // the address of the processed main frame buffer
    int64_t sub;        // the address of the processed sub frame buffer
} face_authenticate_processed_t;

typedef struct face_msg {
    face_msg_type_t type;
    union {
        face_error_t error;
        face_acquired_t acquired;
        face_enroll_t enroll;
        face_removed_t removed;
        face_authenticated_t authenticated;
        face_enumerated_t enumerated;
        face_lockout_t lockout;
        face_enroll_processed_t enroll_processed;
        face_authenticate_processed_t authenticate_processed;
    } data;
} face_msg_t;

/* Callback function type */
typedef void (*face_notify_t)(const face_msg_t *msg);

/* Synchronous operation */
typedef struct face_device {
    /**
     * Common methods of the face device. This *must* be the first member
     * of face_device as users of this structure will cast a hw_device_t
     * to face_device pointer in contexts where it's known
     * the hw_device_t references a face_device.
     */
    struct hw_device_t common;

    /**
     * the handle of face id algorithm
     */
    void* priv_handle;

    /*
     * Client provided callback function to receive notifications.
     * Do not set by hand, use the function below instead.
     */
    face_notify_t notify;

    /*
     * Set notification callback:
     * Registers a user function that would receive notifications from the HAL
     * The call will block if the HAL state machine is in busy state until HAL
     * leaves the busy state.
     *
     * Function return: 0 if callback function is successfully registered
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*set_notify)(struct face_device *dev, face_notify_t notify);

    /*
     * Face pre-enroll enroll request:
     * Generates a unique token to upper layers to indicate the start of an enrollment transaction.
     * This token will be wrapped by security for verification and passed to enroll() for
     * verification before enrollment will be allowed. This is to ensure adding a new face
     * template was preceded by some kind of credential confirmation (e.g. device password).
     *
     * Function return: 0 if get challenge successfully
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*pre_enroll)(struct face_device *dev, uint32_t challengeTimeoutSec, uint64_t *challenge);

    /*
     * Face enroll request:
     * Switches the HAL state machine to collect and store a new face
     * template. Switches back as soon as enroll is complete
     * or after timeout_sec seconds.
     * The face template will be assigned to the group gid. User has a choice
     * to supply the gid or set it to 0 in which case a unique group id will be generated.
     *
     * Function return: 0 if enrollment process can be successfully started
     *                  or a negative number in case of error, generally from the errno.h set.
     *                  A notify() function may be called indicating the error condition.
     */
    int (*enroll)(struct face_device *dev, const hw_auth_token_t *hat,
                  uint32_t timeout_sec, uint32_t *disabledFeatures, size_t count);

    /*
     * send face data to enroll algorithm to process
     *
     * Function return: 0 if process normally
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*do_enroll_process)(struct face_device *dev, int64_t addr, const int32_t *info, int32_t count, const int8_t *byteInfo, int32_t byteCount);

    /*
     * Finishes the enroll operation and invalidates the pre_enroll() generated challenge.
     * This will be called at the end the face enrollment session
     *
     * Function return: 0 if the request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*post_enroll)(struct face_device *dev);

    /*
     * Enable/Disable feature for faceid algo.
     *
     * Function return: 0 if the request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*set_feature)(struct face_device *dev, uint32_t feature, bool enabled, const hw_auth_token_t *hat, uint32_t faceId);

    /*
     * Get feature supported by faceid algorithm.
     *
     * Function return: 0 if get feature successfully
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*get_feature)(struct face_device *dev, uint32_t feature, uint32_t faceId, bool *result);

    /*
     * get_authenticator_id:
     * Returns a token associated with the current face set. This value will
     * change whenever a new face is enrolled, thus creating a new face
     * set.
     *
     * Function return: 0 if get authenticator_id successfully
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*get_authenticator_id)(struct face_device *dev, uint64_t *id);

    /*
     * Cancel pending enroll or authenticate, sending FACE_ERROR_CANCELED
     * to all running clients. Switches the HAL state machine back to the idle state.
     * Unlike enroll_done() doesn't invalidate the pre_enroll() challenge.
     *
     * Function return: 0 if cancel request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*cancel)(struct face_device *dev);

    /*
     * Face enumerate request:
     * Enumerate all templates of current user.
     * notify() will be called with list of fids.
     *
     * Function return: 0 if enumerate request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*enumerate)(struct face_device *dev);

    /*
     * Face remove request:
     * Deletes a face template.
     * Works only within a path set by set_active_group().
     * notify() will be called with details on the template deleted.
     *
     * Function return: 0 if face template(s) can be successfully deleted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*remove)(struct face_device *dev, uint32_t fid);

    /*
     * Restricts the HAL operation to a set of face belonging to a
     * group provided.
     * The caller must provide a path to a storage location within the user's
     * data directory.
     *
     * Function return: 0 on success
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*set_active_group)(struct face_device *dev, int32_t userId,
                            const char *store_path);

    /*
     * Face authenticate request:
     * Authenticates an operation identified by operation_id
     *
     * Function return: 0 on success (the authenticate request is accepted)
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*authenticate)(struct face_device *dev, uint64_t operation_id);

    /*
     * send face data to authenticate algorithm to process
     *
     * Function return: 0 if process normally
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*do_authenticate_process)(struct face_device *dev, int64_t main, int64_t sub, int64_t otp, const int32_t *info, int32_t count, const int8_t *byteInfo, int32_t byteCount);

    /*
     * A hint to the HAL to continue looking for faces.
     *
     * Function return: 0 on success or a negative number in case of error, generally from the errno.h set.
     */
    int (*user_activity)(struct face_device *dev);


    /*
     * Reset lockout for the current user.
     *
     * Function return: 0 on success or a negative number in case of error, generally from the errno.h set.
     */
    int (*reset_lockout)(struct face_device *dev, const hw_auth_token_t *hat);

    /* Reserved for backward binary compatibility */
    void *reserved[4];
} face_device_t;

typedef struct face_module {
    /**
     * Common methods of the face module. This *must* be the first member
     * of face_module as users of this structure will cast a hw_module_t
     * to face_module pointer in contexts where it's known
     * the hw_module_t references a face_module.
     */
    struct hw_module_t common;
} face_module_t;

#endif  /* ANDROID_INCLUDE_HARDWARE_FACE_H */
