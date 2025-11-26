#include "SpeechToText.h"
#ifdef _WIN32
#include <Windows.h>
#include <sapi.h>
#endif

QString SpeechToText::recognizeOnce(){
#ifdef _WIN32
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if(FAILED(hr)) return {};
    ISpRecognizer* pRecognizer = nullptr;
    ISpRecoContext* pContext = nullptr;
    ISpRecoGrammar* pGrammar = nullptr;
    QString resultText;
    do {
        hr = CoCreateInstance(CLSID_SpSharedRecognizer, NULL, CLSCTX_ALL, IID_ISpRecognizer, (void**)&pRecognizer);
        if(FAILED(hr)) break;
        hr = pRecognizer->CreateRecoContext(&pContext); if(FAILED(hr)) break;
        hr = pContext->CreateGrammar(0, &pGrammar); if(FAILED(hr)) break;
        hr = pGrammar->LoadDictation(NULL, SPLO_STATIC); if(FAILED(hr)) break;
        hr = pGrammar->SetDictationState(SPRS_ACTIVE); if(FAILED(hr)) break;
        hr = pContext->SetNotifyWin32Event(); if(FAILED(hr)) break;
        const ULONGLONG interest = SPFEI(SPEI_RECOGNITION);
        hr = pContext->SetInterest(interest, interest); if(FAILED(hr)) break;
        HANDLE hEvent = pContext->GetNotifyEventHandle();
        if(WaitForSingleObject(hEvent, 10000) == WAIT_OBJECT_0){
            SPEVENT evt; ULONG fetched = 0;
            while(SUCCEEDED(pContext->GetEvents(1, &evt, &fetched)) && fetched > 0){
                if(evt.eEventId == SPEI_RECOGNITION){
                    ISpRecoResult* pResult = reinterpret_cast<ISpRecoResult*>(evt.lParam);
                    if(pResult){
                        LPWSTR pwszText = NULL;
                        if(SUCCEEDED(pResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &pwszText, NULL))){
                            resultText = QString::fromWCharArray(pwszText);
                            ::CoTaskMemFree(pwszText);
                        }
                        pResult->Release();
                    }
                    break;
                }
            }
        }
        pGrammar->SetDictationState(SPRS_INACTIVE);
    } while(false);
    if(pGrammar) pGrammar->Release();
    if(pContext) pContext->Release();
    if(pRecognizer) pRecognizer->Release();
    CoUninitialize();
    return resultText;
#else
    return QString();
#endif
}
