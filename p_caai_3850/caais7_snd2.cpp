/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    3.0
Date:		2020-7-28
Description:委外加工费计划发送取消
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caais7_snd2)
//-EP_SYSTEM_HEAD_END
int f_cm_mesac2_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_caais7_snd2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;


	CString		account_period = " ";
	CString		mat_code = " ";
	CString		shift_group = " ";
	CString		dept_code = " ";
	CString		start_time = " ";
	CString		datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	CModel tcaais7("TCAAIS7");


	CString  sqlstr("");
	EIClass bcls_rec1;

	CDbCommand cmd_inq(conn);

	try
	{		
		for (int i = 0; i < bcls_rec->Tables[1].Rows.get_Count(); i++)
		{
			tcaais7.MergeFrom(bcls_rec->Tables[1].Rows[i]);
			EIClass bcls_rec1;
			EIClass bcls_ret1;

			sqlstr = " select t.*,'X' as RETPO from tcaais7 t"
				" where 1=1"
				" and SEND_FLAG='1'"
				" and TAX_RATE_NAME!=' '"
				" and PLAN_NO = @plan_no" //加工单位
				" and TYPE_CODE = @type_code" //加工单位
				" and FARE_INCREASE = @fare_increase" //加工单位
				" and PROC_MODE = @proc_mode" //加工单位
				" and EQU_NO = @equ_no" //加工单位
				" and OUT_UNIT_CODE = @out_unit_code" //加工单位
				" and RECV_DEPT = @recv_dept" //加工单位
				;
			Log::Trace("", "", "MAT_NO=[{0}],plan_no = [{1}],type_code = [{2}],fare_increase = [{3}],proc_mode = [{4}],equ_no = [{5}],out_unit_code = [{6}],recv_dept = [{7}]", tcaais7["MAT_NO"].ToString(), tcaais7["PLAN_NO"].ToString()
				, tcaais7["TYPE_CODE"].ToString(), tcaais7["FARE_INCREASE"].ToString(), tcaais7["PROC_MODE"].ToString(), tcaais7["EQU_NO"].ToString(), tcaais7["OUT_UNIT_CODE"].ToString(), tcaais7["RECV_DEPT"].ToString());
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("mat_no", tcaais7["MAT_NO"].ToString());
			cmd_inq.Parameters.Set("plan_no", tcaais7["PLAN_NO"].ToString());
			cmd_inq.Parameters.Set("type_code", tcaais7["TYPE_CODE"].ToString());
			cmd_inq.Parameters.Set("fare_increase", tcaais7["FARE_INCREASE"].ToString());
			cmd_inq.Parameters.Set("proc_mode", tcaais7["PROC_MODE"].ToString());
			cmd_inq.Parameters.Set("equ_no", tcaais7["EQU_NO"].ToString());
			cmd_inq.Parameters.Set("out_unit_code", tcaais7["OUT_UNIT_CODE"].ToString());
			cmd_inq.Parameters.Set("recv_dept", tcaais7["RECV_DEPT"].ToString());
			cmd_inq.ExecuteQuery(bcls_rec1.Tables[0]);
			cmd_inq.Close();

			

			doFlag = f_cm_mesac2_snd(&bcls_rec1, bcls_ret, conn);
			if (doFlag != 0)
			{
				strcpy(s.msg, "发送SAP失败!");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//发送完成更新日期
			sqlstr = " update tcaais7 set SEND_FLAG = '0'"
				",SEND_TIME = @datenow"
				" where 1=1"
				" and SEND_FLAG='1'"
				" and PLAN_NO = @plan_no" //加工单位
				" and TYPE_CODE = @type_code" //加工单位
				" and FARE_INCREASE = @fare_increase" //加工单位
				" and PROC_MODE = @proc_mode" //加工单位
				" and EQU_NO = @equ_no" //加工单位
				" and OUT_UNIT_CODE = @out_unit_code" //加工单位
				" and RECV_DEPT = @recv_dept" //加工单位
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("mat_no", tcaais7["MAT_NO"].ToString());
			cmd_inq.Parameters.Set("plan_no", tcaais7["PLAN_NO"].ToString());
			cmd_inq.Parameters.Set("type_code", tcaais7["TYPE_CODE"].ToString());
			cmd_inq.Parameters.Set("fare_increase", tcaais7["FARE_INCREASE"].ToString());
			cmd_inq.Parameters.Set("proc_mode", tcaais7["PROC_MODE"].ToString());
			cmd_inq.Parameters.Set("equ_no", tcaais7["EQU_NO"].ToString());
			cmd_inq.Parameters.Set("out_unit_code", tcaais7["OUT_UNIT_CODE"].ToString());
			cmd_inq.Parameters.Set("recv_dept", tcaais7["RECV_DEPT"].ToString());
			cmd_inq.Parameters.Set("datenow", datenow);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}