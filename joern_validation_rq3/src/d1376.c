views::View* TrayPower::CreateTrayView(user::LoginStatus status) {
  PowerSupplyStatus power_status =
      ash::Shell::GetInstance()->tray_delegate()->GetPowerSupplyStatus();
  power_tray_.reset(new tray::PowerTrayView());
  power_tray_->UpdatePowerStatus(power_status);
  return power_tray_.get();
}
