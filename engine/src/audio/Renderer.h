//
// C++ Interface: Audio::SceneManager
//
#ifndef __AUDIO_RENDERER_H__INCLUDED__
#define __AUDIO_RENDERER_H__INCLUDED__

#include <string>
#include <map>
#include <set>
#include "Exceptions.h"
#include "Singleton.h"
#include "Types.h"
#include "Format.h"

#include "vsfilesystem.h"

namespace Audio {

    // Some forwards
    class Source;
    class Sound;
    class Listener;


    /**
     * Audio Renderer interface.
     *
     * @remarks Audio renderer implementations will implement this interface to communicate
     *      with SceneManager s.
     *      @par Renderers are also the factories for sound resources, since most APIs already
     *      handle sample storage and other sound resource aspects.
     *      @par The interface is rather simple since most operations are done through 
     *      RenderableSource proxies.
     *
     */
    class Renderer 
    {
    private:
        Scalar meterDistance;
        Scalar dopplerFactor;
        Format outputFormat;
    
    public:
        /** Initialize the renderer with default or config-driven settings.
         * @remarks End-users might want to use specific constructors of specific renderers.
         */
        Renderer();
        
        virtual ~Renderer();
        
        /** Create a sound, from the stream of the specified name.
         * @param name The path of the soundfile.
         * @param type The file type (needed by the filesystem).
         * @param straeming If true, requests for a streaming sound - one that is not loaded to
         *      memory for playback, but rather read on-demand during playback.
         * @remarks The name is the path of a soundstream as it would be passed to the CodecRegistry.
         *      @par Streaming sounds can also be created, which depending on the renderer implementation
         *      may or may not result in a different kind of sound resource. Most notably, the difference
         *      may depend on runtime-dependant state. For instance, it is quite reasonable that a
         *      streaming sound won't be created if the same sound file exists in a non-streaming
         *      form.
         * @see CodecRegistry
         */
        virtual SharedPtr<Sound> getSound(
            const std::string &name, 
            VSFileSystem::VSFileType type = VSFileSystem::UnknownFile, 
            bool streaming = false) = 0;
        
        /** Return whether the specified sound has been created using this renderer or not */
        virtual bool owns(SharedPtr<Sound> sound) = 0;
        
        /** Attach a source to this renderer
         * @remarks A source may only be attached to one renderer. If the source was attached already,
         *      an exception will be thrown.
         *      @par Attachment may mean resource allocation. Either immediate or deferred. So it may
         *      fail if resources are scarce.
         */
        virtual void attach(SharedPtr<Source> source) = 0;
        
        /** Attach a listener to this renderer
         * @remarks A listener may only be attached to one renderer. If the listener was attached already,
         *      an exception will be thrown.
         *      @par Attachment may mean resource allocation. Either immediate or deferred. So it may
         *      fail if resources are scarce.
         */
        virtual void attach(SharedPtr<Listener> listener) = 0;
        
        /** Detach a source from this renderer.
         * @remarks Immediately frees any allocated resources.
         */
        virtual void detach(SharedPtr<Source> source) throw() = 0;
        
        /** Detach a listener from this renderer.
         * @remarks Immediately frees any allocated resources.
         */
        virtual void detach(SharedPtr<Listener> listener) throw() = 0;
        
        
        
        /** Sets the distance in world units that represents one meter.
         * @remarks This reference distance is required by environmental effect processing
         *      to accurately account for distance factors beyond simple gain falloff.
         */
        virtual void setMeterDistance(Scalar distance) throw();
        
        /** Gets the distance in world units that represents one meter.
         * @see setMeterDistance
         */
        virtual Scalar getMeterDistance() const throw();
        
        
        /** Sets how much the doppler effect will be accounted for.
         * @remarks This sets a semi-opaque value which controls how much of the doppler
         *      effect will be simulated. All that is required is that 0 maps to fully
         *      disabled doppler effect, 1 maps to a realistic effect, in between
         *      a disimulated effect, and above 1 an exaggerated effect.
         *          The spec is purposefully vague on the specifics.
         */
        virtual void setDopplerFactor(Scalar factor) throw();
        
        /** Gets how much the doppler effect will be accounted for.
         * @see setDopplerFactor
         */
        virtual Scalar getDopplerFactor() const throw();
        
        
        
        /** Sets the (preferred) output format.
         * @remarks Renderers are encouraged to set their effective output format
         *      to the closest "better" format, where better is defined as either
         *      having heigher sampling frequency, bit depth, or number of channels.
         *          The effective output format, if known, must be reflected in
         *      subsequent calls to getOutputFormat.
         */
        virtual void setOutputFormat(const Format &format);
        
        /** Gets the distance in world units that represents one meter.
         * @see setMeterDistance
         */
        virtual const Format& getOutputFormat() const throw();
        
        /**
         * Begins a transaction
         * @remarks state changes will be piled up and applied
         *      at commit. This is, though, an optional feature: renderers may choose
         *      not to implement it, and perform state changes immediately. In any
         *      case, the result ought to be the same.
         */
        virtual void beginTransaction();
        
        /** @see begin() */
        virtual void commitTransaction();
    };

};

#endif//__AUDIO_RENDERER_H__INCLUDED__
