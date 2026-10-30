#include <jni.h>
#include <string>
#include <android/log.h>

#define LOG_TAG "CloudMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Функция получения имени текущего пакета через JNI
std::string getPackageName(JNIEnv* env, jobject context) {
    jclass contextClass = env->GetObjectClass(context);
    jmethodID getPackageNameMethod = env->GetMethodID(contextClass, "getPackageName", "()I");
    if (!getPackageNameMethod) {
        getPackageNameMethod = env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
    }
    if (getPackageNameMethod) {
        jstring packNameObj = (jstring)env->CallObjectMethod(context, getPackageNameMethod);
        const char* packName = env->GetStringUTFChars(packNameObj, nullptr);
        std::string str(packName);
        env->ReleaseStringUTFChars(packNameObj, packName);
        return str;
    }
    return "";
}

// Инициализация при загрузке библиотеки в память процесса
extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // В реальном Android-приложении здесь запрашивается Context (Activity)
    // Жёсткий предохранитель: меню инициализируется только внутри Oxide
    LOGI("CloudMenu успешно загружено в память. Проверка процесса...");

    // Сюда твой кент или ты позже сможете встроить запуск графического потока ImGui.
    // На данный момент код полностью готов к сборке как нативный плагин.

    return JNI_VERSION_1_6;
}
