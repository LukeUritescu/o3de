/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzFramework/Viewport/ViewportBus.h>
#include <AzFramework/Windowing/WindowBus.h>
#include <Atom/RPI.Public/WindowContext.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/SceneBus.h>
#include <AzCore/EBus/Event.h>

namespace AZ
{
    namespace RPI
    {
        class ViewportContextManager;

        //! ViewportContext wraps a native window and represents a minimal viewport
        //! in which a scene is rendered on-screen.
        //! ViewportContexts are registered on creation to allow consumers to listen to notifications
        //! and manage the view stack for a given viewport.
        class ViewportContext
            : public SceneNotificationBus::Handler
            , public AzFramework::WindowNotificationBus::Handler
            , public AzFramework::ViewportRequestBus::Handler
        {
        public:
            AZ_CLASS_ALLOCATOR(ViewportContext, AZ::SystemAllocator, 0);

            //! Used by ViewportContextManager, use AZ::Interface<ViewportContextRequestsInterface>::Get()->CreateViewportContext to create a viewport
            //! context from outside of the ViewportContextManager.
            ViewportContext(ViewportContextManager* manager, AzFramework::ViewportId id, const AZ::Name& name, RHI::Device& device, AzFramework::NativeWindowHandle nativeWindow, ScenePtr renderScene);
            ~ViewportContext();

            //! Gets an opaque ID that can be used to uniquely identify this ViewportContext.
            AzFramework::ViewportId GetId() const;
            //! Convenience method, gets the window handle associated with this viewport's window manager.
            AzFramework::NativeWindowHandle GetWindowHandle() const;
            //! Gets the window context associated with this viewport.
            WindowContextSharedPtr GetWindowContext();
            //! Gets the root scene (if any) associated with this viewport.
            ScenePtr GetRenderScene();
            //! Gets the current render pipeline associated with our WindowContext, if there is one.
            RenderPipelinePtr GetCurrentPipeline();
            //! Sets the root scene associated with this viewport.
            //! This does not provide a default render pipeline, one must be provided to enable rendering.
            void SetRenderScene(ScenePtr scene);

            //! Gets the maximum frame rate this viewport context's pipeline can render at, 0 for unlimited.
            //! The target framerate for the pipeline will be determined by this frame limit and the
            //! vsync settings for the current window.
            float GetFpsLimit() const;

            //! Sets the maximum frame rate this viewport context's pipeline can render at, 0 for unlimited.
            //! The target framerate for the pipeline will be determined by this frame limit and the
            //! vsync settings for the current window.
            void SetFpsLimit(float fpsLimit);

            //! Gets the target frame rate for this viewport context.
            //! This returns the lowest of either the current VSync refresh rate
            //! or 0 for an unlimited frame rate (if there's no FPS limit and vsync is off).
            float GetTargetFrameRate() const;

            //! Gets the current name of this ViewportContext.
            //! This name is used to tie this ViewportContext to its View stack, and ViewportContexts may be
            //! renamed via AZ::RPI::ViewportContextRequests::Get()->RenameViewportContext(...).
            AZ::Name GetName() const;

            //! Gets the default view associated with this ViewportContext.
            //! Alternatively, use  AZ::RPI::ViewportContextRequests::Get()->GetCurrentView().
            ViewPtr GetDefaultView();
            ConstViewPtr GetDefaultView() const;

            //! Gets the current size of the viewport.
            //! This value is cached and updated on-demand, so may be efficiently queried.
            AzFramework::WindowSize GetViewportSize() const;

            //! Gets the screen DPI scaling factor.
            //! This value is cached and updated on-demand, so may be efficiently queried.
            //! \see AzFramework::WindowRequests::GetDpiScaleFactor
            float GetDpiScalingFactor() const;

            //! Gets the current vsync interval, as a divisor of the current refresh rate.
            //! A value of 0 indicates that vsync is disabled.
            uint32_t GetVsyncInterval() const;

            //! Gets the current display refresh rate, in frames per second.
            uint32_t GetRefreshRate() const;

            // SceneNotificationBus interface overrides...
            //! Ensures our default view remains set when our scene's render pipelines are modified.
            void OnRenderPipelineAdded(RenderPipelinePtr pipeline) override;
            //! Ensures our default view remains set when our scene's render pipelines are modified.
            void OnRenderPipelineRemoved(RenderPipeline* pipeline) override;

            // WindowNotificationBus interface overrides...
            //! Used to fire a notification when our window resizes.
            void OnWindowResized(uint32_t width, uint32_t height) override;
            //! Used to fire a notification when our window DPI changes.
            void OnDpiScaleFactorChanged(float dpiScaleFactor) override;
            //! Used to fire a notification when our vsync interval changes.
            void OnVsyncIntervalChanged(uint32_t interval) override;
            //! Used to fire a notification when our refresh rate changes.
            void OnRefreshRateChanged(uint32_t refreshRate) override;

            using SizeChangedEvent = AZ::Event<AzFramework::WindowSize>;
            //! Notifies consumers when the viewport size has changed.
            //! Alternatively, connect to ViewportContextNotificationsBus and listen to ViewportContextNotifications::OnViewportSizeChanged.
            void ConnectSizeChangedHandler(SizeChangedEvent::Handler& handler);

            using ScalarChangedEvent = AZ::Event<float>;
            //! Notifies consumers when the viewport DPI scaling ratio has changed.
            //! Alternatively, connect to ViewportContextNotificationsBus and listen to ViewportContextNotifications::OnViewportDpiScalingChanged.
            void ConnectDpiScalingFactorChangedHandler(ScalarChangedEvent::Handler& handler);

            using UintChangedEvent = AZ::Event<uint32_t>;
            //! Notifies consumers when the vsync interval has changed.
            void ConnectVsyncIntervalChangedHandler(UintChangedEvent::Handler& handler);
            //! Notifies consumers when the refresh rate has changed.
            void ConnectRefreshRateChangedHandler(UintChangedEvent::Handler& handler);

            using MatrixChangedEvent = AZ::Event<const AZ::Matrix4x4&>;
            //! Notifies consumers when the view matrix has changed.
            void ConnectViewMatrixChangedHandler(MatrixChangedEvent::Handler& handler);
            //! Notifies consumers when the projection matrix has changed.
            void ConnectProjectionMatrixChangedHandler(MatrixChangedEvent::Handler& handler);

            using SceneChangedEvent = AZ::Event<ScenePtr>;
            //! Notifies consumers when the render scene has changed.
            void ConnectSceneChangedHandler(SceneChangedEvent::Handler& handler);

            using PipelineChangedEvent = AZ::Event<RenderPipelinePtr>;
            //! Notifies consumers when the current pipeline associated with our window has changed.
            void ConnectCurrentPipelineChangedHandler(PipelineChangedEvent::Handler& handler);

            using ViewChangedEvent = AZ::Event<ViewPtr>;
            //! Notifies consumers when the default view has changed.
            void ConnectDefaultViewChangedHandler(ViewChangedEvent::Handler& handler);

            using ViewportIdEvent = AZ::Event<AzFramework::ViewportId>;
            //! Notifies consumers when this ViewportContext is about to be destroyed.
            void ConnectAboutToBeDestroyedHandler(ViewportIdEvent::Handler& handler);

            // ViewportRequestBus interface overrides...
            //! Gets the current camera's view matrix.
            const AZ::Matrix4x4& GetCameraViewMatrix() const override;
            //! Sets the current camera's view matrix.
            void SetCameraViewMatrix(const AZ::Matrix4x4& matrix) override;
            //! Gets the current camera's projection matrix.
            const AZ::Matrix4x4& GetCameraProjectionMatrix() const override;
            //! Sets the current camera's projection matrix.
            void SetCameraProjectionMatrix(const AZ::Matrix4x4& matrix) override;
            //! Convenience method, gets the AZ::Transform corresponding to this camera's view matrix.
            AZ::Transform GetCameraTransform() const override;
            //! Convenience method, sets the camera's view matrix from this AZ::Transform.
            void SetCameraTransform(const AZ::Transform& transform) override;

        private:
            // Used by the manager to set the current default camera.
            void SetDefaultView(ViewPtr view);
            // Ensures our render pipeline's default camera matches ours.
            void UpdatePipelineView();
            // Ensures our render pipeline refresh rate matches our refresh rate.
            void UpdatePipelineRefreshRate();
            // Resets the current pipeline reference and ensures pipeline events are disconnected.
            void ResetCurrentPipeline();

            ScenePtr m_rootScene;
            WindowContextSharedPtr m_windowContext;
            ViewPtr m_defaultView;
            AzFramework::WindowSize m_viewportSize;
            float m_viewportDpiScaleFactor = 1.0f;
            uint32_t m_vsyncInterval = 1;
            uint32_t m_refreshRate = 60;
            float m_fpsLimit = 0.f;

            SizeChangedEvent m_sizeChangedEvent;
            ScalarChangedEvent m_dpiScalingFactorChangedEvent;
            UintChangedEvent m_vsyncIntervalChangedEvent;
            UintChangedEvent m_refreshRateChangedEvent;
            MatrixChangedEvent m_viewMatrixChangedEvent;
            MatrixChangedEvent::Handler m_onViewMatrixChangedHandler;
            MatrixChangedEvent m_projectionMatrixChangedEvent;
            MatrixChangedEvent::Handler m_onProjectionMatrixChangedHandler;
            SceneChangedEvent m_sceneChangedEvent;
            PipelineChangedEvent m_currentPipelineChangedEvent;
            ViewChangedEvent m_defaultViewChangedEvent;
            ViewportIdEvent m_aboutToBeDestroyedEvent;

            AZ::Event<>::Handler m_prepareFrameHandler;
            AZ::Event<>::Handler m_endFrameHandler;

            ViewportContextManager* m_manager;
            RenderPipelinePtr m_currentPipeline;
            Name m_name;
            AzFramework::ViewportId m_id;

            friend class ViewportContextManager;
        };
    } // namespace RPI
} // namespace AZ
